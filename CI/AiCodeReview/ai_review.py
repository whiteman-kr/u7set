import json
import os
import re
import subprocess
import sys
from dataclasses import dataclass
from pathlib import PurePosixPath
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
OLLAMA_MODEL = os.environ.get("OLLAMA_MODEL", "qwen2.5-coder:14b")
REVIEWABLE_CPP_EXTENSIONS = {".cpp", ".h", ".hpp"}
MAX_CONTEXT_FILE_BYTES = int(os.environ.get("AI_REVIEW_MAX_FILE_BYTES", "40000"))
MAX_CONTEXT_BYTES = int(os.environ.get("AI_REVIEW_MAX_CONTEXT_BYTES", "120000"))
GENERATED_DIRECTORY_NAMES = {"build", "generated", "autogen", "cmakefiles"}
GENERATED_FILE_PREFIXES = ("moc_", "ui_", "qrc_")
GENERATED_FILE_SUFFIXES = (".generated.cpp", ".generated.h", ".generated.hpp", ".pb.cpp", ".pb.h", ".pb.hpp")


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


def get_changed_paths() -> list[str]:
    if not CI_BEFORE_SHA or CI_BEFORE_SHA == ZERO_SHA:
        command = ["git", "diff-tree", "--root", "--no-commit-id", "--name-only", "-r", "HEAD"]
    else:
        command = ["git", "diff", "--name-only", f"{CI_BEFORE_SHA}...{CI_COMMIT_SHA}"]

    result = subprocess.run(
        command,
        check=True,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
    )
    return [path for path in result.stdout.splitlines() if path]


def get_commit_file(path: str) -> Optional[bytes]:
    result = subprocess.run(
        ["git", "show", f"{CI_COMMIT_SHA}:{path}"],
        check=False,
        capture_output=True,
    )
    if result.returncode != 0:
        return None
    return result.stdout


def is_reviewable_cpp_path(path: str) -> bool:
    normalized_path = path.replace("\\", "/")
    path_parts = PurePosixPath(normalized_path).parts
    file_name = PurePosixPath(normalized_path).name.lower()
    suffix = PurePosixPath(normalized_path).suffix.lower()

    if suffix not in REVIEWABLE_CPP_EXTENSIONS:
        return False
    if any(part.lower() in GENERATED_DIRECTORY_NAMES for part in path_parts):
        return False
    if file_name.startswith(GENERATED_FILE_PREFIXES):
        return False
    return not file_name.endswith(GENERATED_FILE_SUFFIXES)


def get_review_context_paths(changed_paths: list[str]) -> list[str]:
    repository_paths = subprocess.run(
        ["git", "ls-tree", "-r", "--name-only", CI_COMMIT_SHA],
        check=True,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
    ).stdout.splitlines()
    repository_path_set = set(repository_paths)
    context_paths = {path for path in changed_paths if is_reviewable_cpp_path(path)}

    for changed_path in changed_paths:
        if PurePosixPath(changed_path).suffix.lower() != ".cpp":
            continue

        changed_file = PurePosixPath(changed_path)
        header_names = {f"{changed_file.stem}.h", f"{changed_file.stem}.hpp"}
        same_directory = changed_file.parent
        candidates = [
            str(same_directory / name) if str(same_directory) != "." else name
            for name in sorted(header_names)
        ]
        candidates.extend(
            path for path in repository_paths
            if PurePosixPath(path).name in header_names
        )
        context_paths.update(
            path for path in candidates
            if path in repository_path_set and is_reviewable_cpp_path(path)
        )

    return sorted(context_paths)


def build_review_context(diff_text: str) -> str:
    context_paths = get_review_context_paths(get_changed_paths())
    sections = ["## COMPLETE GIT DIFF\n\n```diff\n" + diff_text + "\n```"]
    included_paths: list[str] = []
    omitted_files: list[str] = []
    context_bytes = len(diff_text.encode("utf-8", errors="replace"))

    if context_bytes > MAX_CONTEXT_BYTES:
        print(
            f"Diff is {context_bytes} bytes, exceeding the total context limit "
            f"of {MAX_CONTEXT_BYTES} bytes; omitting complete-file context."
        )

    for path in context_paths:
        content = get_commit_file(path)
        if content is None:
            continue
        if len(content) > MAX_CONTEXT_FILE_BYTES:
            omitted_files.append(f"{path} (file limit: {len(content)} bytes)")
            continue
        if context_bytes + len(content) > MAX_CONTEXT_BYTES:
            omitted_files.append(f"{path} (total context limit)")
            continue

        text = content.decode("utf-8", errors="replace")
        sections.append(f"## COMPLETE FILE: {path}\n\n```cpp\n{text}\n```")
        included_paths.append(path)
        context_bytes += len(content)

    if included_paths:
        print(f"Including complete C++ file context: {', '.join(included_paths)}")
    if omitted_files:
        print(f"Omitting C++ context files: {', '.join(omitted_files)}")

    return "\n\n".join(sections)


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


def get_ai_review(review_context: str) -> str:
    prompt = (
        "You are an expert C++23, Qt6 code reviewer. Review the supplied Git diff and complete file context for defects, "
        "undefined behavior, memory/resource leaks, security risks, and clearly obsolete C++ idioms. "
        "Use the complete files to understand declarations, surrounding functions, and ownership, but report only issues caused by the supplied diff. "
        "Provide actionable Markdown feedback. The summary_markdown must explain each finding, not just say that issues exist. "
        "For every critical issue, state the exact file path, line number, defect, runtime impact, and recommended correction. "
        "A critical issue can cause a security vulnerability, data loss or corruption, a crash, "
        "undefined behavior, or a major functional failure. "
        "Return one JSON object only, with this exact shape: "
        '{"result":"OK or CRITICAL","summary_markdown":"Markdown review",'
        '"critical_issues":[{"path":"repository/relative/file.cpp","line":123,'
        '"message":"concise Markdown comment"}]}. '
        "Set result to CRITICAL exactly when critical_issues is non-empty. "
        "Each critical issue must cite an added or modified line on the new side of the diff. "
        "If the faulty code is unchanged context, cite the nearest added or modified line in that file. "
        "Use an empty critical_issues array when result is OK.\n\n"
        f"{review_context}"
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


def validate_issue_location(issue: CriticalIssue) -> None:
    path_parts = issue.path.split("/")
    if issue.path.startswith("/") or "\\" in issue.path or any(part in {"", ".", ".."} for part in path_parts):
        raise ValueError(f"Critical issue references an invalid path: {issue.path}")

    result = subprocess.run(
        ["git", "show", f"{CI_COMMIT_SHA}:{issue.path}"],
        check=False,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
    )
    if result.returncode != 0:
        raise ValueError(f"Critical issue references a file absent from the commit: {issue.path}")

    line_count = len(result.stdout.splitlines())
    if issue.line < 1 or issue.line > line_count:
        raise ValueError(f"Critical issue references an invalid line: {issue.path}:{issue.line}")


def attach_to_changed_line(issue: CriticalIssue, changed_lines: dict[str, set[int]]) -> CriticalIssue:
    file_changed_lines = changed_lines.get(issue.path)
    if not file_changed_lines:
        raise ValueError(f"Critical issue references a file without changed lines: {issue.path}")

    attached_line = min(file_changed_lines, key=lambda line: abs(line - issue.line))
    if attached_line == issue.line:
        return issue

    message = (
        f"**AI-reported location:** `{issue.path}:{issue.line}`. "
        f"Attached to the nearest changed line, `{issue.path}:{attached_line}`.\n\n{issue.message}"
    )
    return CriticalIssue(issue.path, attached_line, message)


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

        issue = CriticalIssue(path, line, message.strip())
        validate_issue_location(issue)
        issues.append(attach_to_changed_line(issue, changed_lines))

    if (result == "CRITICAL") != bool(issues):
        raise ValueError("Ollama result does not match the listed critical issues.")

    return summary.strip(), issues


def build_final_report(summary: str, critical_issues: list[CriticalIssue]) -> str:
    report_lines = ["## AI Code Review", summary.strip()]

    if not critical_issues:
        report_lines.extend(["", "**Result:** OK - no critical issues found."])
        return "\n\n".join(report_lines)

    report_lines.extend(["", "## Critical issues"])
    for issue in critical_issues:
        report_lines.extend([
            "",
            f"### `{issue.path}:{issue.line}`",
            issue.message,
        ])

    return "\n".join(report_lines).strip()


def get_commit_comments_url() -> str:
    return f"{CI_API_V4_URL}/projects/{CI_PROJECT_ID}/repository/commits/{CI_COMMIT_SHA}/comments"


def post_commit_comment(note: str, issue: Optional[CriticalIssue] = None) -> None:
    payload = {"note": note}
    if issue:
        payload.update({"path": issue.path, "line": issue.line, "line_type": "new"})

    response = requests.post(
        get_commit_comments_url(),
        headers={"PRIVATE-TOKEN": GITLAB_TOKEN},
        data=payload,
        timeout=60,
    )
    response.raise_for_status()


def main() -> int:
    require_environment()
    diff = get_git_diff()
    if not diff.strip():
        print("No changes detected in diff. Skipping AI review.")
        return 0

    print("Sending diff and C++ file context to Ollama on host...")
    review_context = build_review_context(diff)
    summary, critical_issues = parse_review(get_ai_review(review_context), get_changed_new_lines(diff))
    final_report = build_final_report(summary, critical_issues)
    print("\n===== AI Code Review (Markdown) =====")
    print(final_report)
    print("===== End AI Code Review =====\n")

    post_commit_comment(final_report)
    for issue in critical_issues:
        mention = f"@{GITLAB_REVIEW_MENTION.lstrip('@')} " if GITLAB_REVIEW_MENTION else ""
        note = (
            f"{mention}### Critical AI Code Review\n\n"
            f"**Location:** `{issue.path}:{issue.line}`\n\n"
            f"{issue.message}"
        )
        post_commit_comment(note, issue)
        print(f"Posted critical inline comment: {issue.path}:{issue.line}")

    if critical_issues:
        print("Critical issue found. Marking this allowed-to-fail job as a warning.")
        return 1

    print("AI Review complete: OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
