import json
import os
import re
import subprocess
import sys
from dataclasses import dataclass
from typing import Optional

import requests

ZERO_SHA = "0" * 40
HUNK_HEADER = re.compile(r"^@@ -\d+(?:,\d+)? \+(\d+)(?:,\d+)? @@")

GITLAB_TOKEN = os.environ.get("GITLAB_API_TOKEN")
CI_PROJECT_ID = os.environ.get("CI_PROJECT_ID")
CI_COMMIT_SHA = os.environ.get("CI_COMMIT_SHA")
CI_BEFORE_SHA = os.environ.get("CI_COMMIT_BEFORE_SHA")
CI_API_V4_URL = os.environ.get("CI_API_V4_URL")
GITLAB_REVIEW_MENTION = os.environ.get("GITLAB_REVIEW_MENTION", "").strip()
OLLAMA_URL = os.environ.get("OLLAMA_URL", "http://127.0.0.1:11434")
OLLAMA_TIMEOUT = int(os.environ.get("OLLAMA_TIMEOUT", "1200"))
OLLAMA_MODEL = os.environ.get("OLLAMA_MODEL", "qwen2.5-coder:32b")


@dataclass(frozen=True)
class CriticalIssue:
    path: str
    line: int
    message: str


def require_environment() -> None:
    required = {
        "GITLAB_API_TOKEN": GITLAB_TOKEN,
        "CI_PROJECT_ID": CI_PROJECT_ID,
        "CI_COMMIT_SHA": CI_COMMIT_SHA,
        "CI_API_V4_URL": CI_API_V4_URL,
    }
    missing = [name for name, value in required.items() if not value]
    if missing:
        raise RuntimeError(f"Missing required environment variable(s): {', '.join(missing)}")


def get_git_diff() -> str:
    if not CI_BEFORE_SHA or CI_BEFORE_SHA == ZERO_SHA:
        command = ["git", "diff-tree", "--root", "--no-commit-id", "--patch", "HEAD"]
    else:
        command = ["git", "diff", f"{CI_BEFORE_SHA}...{CI_COMMIT_SHA}"]

    result = subprocess.run(
        command,
        check=True,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
    )
    return result.stdout


def get_changed_new_lines(diff_text: str) -> dict[str, set[int]]:
    changed_lines: dict[str, set[int]] = {}
    current_path: Optional[str] = None
    new_line: Optional[int] = None

    for diff_line in diff_text.splitlines():
        if diff_line.startswith("+++ b/"):
            current_path = diff_line[6:]
            changed_lines.setdefault(current_path, set())
            new_line = None
            continue

        hunk_match = HUNK_HEADER.match(diff_line)
        if hunk_match:
            new_line = int(hunk_match.group(1))
            continue

        if current_path is None or new_line is None:
            continue

        if diff_line.startswith("+"):
            changed_lines[current_path].add(new_line)
            new_line += 1
        elif diff_line.startswith(" "):
            new_line += 1

    return changed_lines


def get_ai_review(diff_text: str) -> str:
    prompt = (
        "You are an expert C++23 code reviewer. Review the supplied Git diff for defects, "
        "undefined behavior, memory/resource leaks, security risks, and clearly obsolete C++ idioms. "
        "Provide concise, actionable Markdown feedback with source file names and line numbers. "
        "A critical issue can cause a security vulnerability, data loss or corruption, a crash, "
        "undefined behavior, or a major functional failure. "
        "Return one JSON object only, with this exact shape: "
        '{"result":"OK or CRITICAL","summary_markdown":"Markdown review",'
        '"critical_issues":[{"path":"repository/relative/file.cpp","line":123,'
        '"message":"concise Markdown comment"}]}. '
        "Set result to CRITICAL exactly when critical_issues is non-empty. "
        "Each critical issue must refer only to an added or modified line on the new side of the diff. "
        "Use an empty critical_issues array when result is OK.\n\n"
        f"```diff\n{diff_text}\n```"
    )
    payload = {
        "model": OLLAMA_MODEL,
        "prompt": prompt,
        "format": "json",
        "stream": False,
        "keep_alive": "2m",
    }
    response = requests.post(f"{OLLAMA_URL}/api/generate", json=payload, timeout=OLLAMA_TIMEOUT)
    response.raise_for_status()
    return response.json()["response"]


def parse_review(review_text: str, changed_lines: dict[str, set[int]]) -> tuple[str, list[CriticalIssue]]:
    try:
        review = json.loads(review_text)
    except json.JSONDecodeError as error:
        raise ValueError("Ollama returned invalid JSON.") from error

    result = review.get("result")
    summary = review.get("summary_markdown")
    raw_issues = review.get("critical_issues")
    if result not in {"OK", "CRITICAL"} or not isinstance(summary, str) or not isinstance(raw_issues, list):
        raise ValueError("Ollama review does not match the required JSON structure.")

    issues: list[CriticalIssue] = []
    for raw_issue in raw_issues:
        if not isinstance(raw_issue, dict):
            raise ValueError("Ollama returned an invalid critical issue.")

        path = raw_issue.get("path")
        line = raw_issue.get("line")
        message = raw_issue.get("message")
        if not isinstance(path, str) or isinstance(line, bool) or not isinstance(line, int) or not isinstance(message, str):
            raise ValueError("Ollama returned an invalid critical issue.")
        if path not in changed_lines or line not in changed_lines[path]:
            raise ValueError(f"Critical issue references unchanged or unknown location: {path}:{line}")

        issues.append(CriticalIssue(path, line, message.strip()))

    if (result == "CRITICAL") != bool(issues):
        raise ValueError("Ollama result does not match the listed critical issues.")

    return summary.strip(), issues


def get_commit_comments_url() -> str:
    return f"{CI_API_V4_URL}/projects/{CI_PROJECT_ID}/repository/commits/{CI_COMMIT_SHA}/comments"


def post_commit_comment(note: str, issue: Optional[CriticalIssue] = None) -> None:
    payload = {"note": note}
    if issue:
        payload.update({"path": issue.path, "line": issue.line, "line_type": "new"})

    response = requests.post(
        get_commit_comments_url(),
        headers={"PRIVATE-TOKEN": GITLAB_TOKEN},
        json=payload,
        timeout=60,
    )
    response.raise_for_status()


def main() -> int:
    require_environment()
    diff = get_git_diff()
    if not diff.strip():
        print("No changes detected in diff. Skipping AI review.")
        return 0

    print("Sending diff to Ollama on host...")
    summary, critical_issues = parse_review(get_ai_review(diff), get_changed_new_lines(diff))
    print("\n===== AI Code Review (Markdown) =====")
    print(summary)
    print("===== End AI Code Review =====\n")

    post_commit_comment(f"### AI Code Review\n\n{summary}")
    for issue in critical_issues:
        mention = f"@{GITLAB_REVIEW_MENTION.lstrip('@')} " if GITLAB_REVIEW_MENTION else ""
        note = f"{mention}### Critical AI Code Review\n\n{issue.message}"
        post_commit_comment(note, issue)
        print(f"Posted critical inline comment: {issue.path}:{issue.line}")

    if critical_issues:
        print("Critical issue found. Marking this allowed-to-fail job as a warning.")
        return 1

    print("AI Review complete: OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
