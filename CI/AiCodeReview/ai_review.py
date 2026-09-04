import os
import re
import subprocess
import sys

import requests

GITLAB_TOKEN = os.environ.get("GITLAB_API_TOKEN")
CI_PROJECT_ID = os.environ.get("CI_PROJECT_ID")
CI_COMMIT_SHA = os.environ.get("CI_COMMIT_SHA")
CI_BEFORE_SHA = os.environ.get("CI_COMMIT_BEFORE_SHA")
CI_API_V4_URL = os.environ.get("CI_API_V4_URL")
OLLAMA_URL = os.environ.get("OLLAMA_URL", "http://127.0.0.1:11434")
OLLAMA_TIMEOUT = int(os.environ.get("OLLAMA_TIMEOUT", "1200"))
OLLAMA_MODEL = os.environ.get("OLLAMA_MODEL", "qwen2.5-coder:32b")

def get_git_diff():
    # If it's a brand new branch push, BEFORE_SHA will be all zeros
    if not CI_BEFORE_SHA or CI_BEFORE_SHA == "0000000000000000000000000000000000000000":
        # Compare current commit against its immediate parent
        cmd = ['git', 'diff', 'HEAD~1', 'HEAD']
    else:
        # Compare previous commit on branch to the new pushed commit
        cmd = ['git', 'diff', f'{CI_BEFORE_SHA}...{CI_COMMIT_SHA}']
        
    result = subprocess.run(cmd, capture_output=True, text=True)
    return result.stdout

def get_ai_review(diff_text):
    prompt = (
        "You are an expert C++ code reviewer enforcing modern C++23 standards. "
        "Review the following code changes. "
        "Identify bugs, memory leaks, and undefined behavior. "
        "Flag absolutely outdated C++ idioms if any. "
        "Keep the review concise and provide short code examples for your suggestions. Organize feedback by source files, add line numbers."
        "A critical issue is one that can cause a security vulnerability, data loss or corruption, "
        "crash, undefined behavior, or a major functional failure. "
        "End the review with exactly one final line: REVIEW_RESULT: CRITICAL if any critical issue "
        "is found; otherwise end with exactly: REVIEW_RESULT: OK.\n\n"
        f"```diff\n{diff_text}\n```"
    )
    
    payload = {
        "model": OLLAMA_MODEL,
        "prompt": prompt,
        "stream": False,
        "keep_alive": "2m"
    }
    
    response = requests.post(f"{OLLAMA_URL}/api/generate", json=payload, timeout=OLLAMA_TIMEOUT)
    response.raise_for_status()
    return response.json()['response']


def parse_review_result(review_text):
    match = re.search(r"\n?REVIEW_RESULT:\s*(CRITICAL|OK)\s*$", review_text)
    if not match:
        raise ValueError("Ollama review did not include a valid final REVIEW_RESULT marker.")

    return review_text[:match.start()].rstrip(), match.group(1)


def post_comment_to_commit(review_text):
    url = f"{CI_API_V4_URL}/projects/{CI_PROJECT_ID}/repository/commits/{CI_COMMIT_SHA}/comments"
    headers = {"PRIVATE-TOKEN": GITLAB_TOKEN}
    payload = {"note": f"### 🤖 AI Code Review\n\n{review_text}"}
    
    response = requests.post(url, headers=headers, json=payload)
    response.raise_for_status()

if __name__ == "__main__":
    diff = get_git_diff()

    if diff and diff.strip():
        print("Sending diff to Ollama on host...")
        review, review_result = parse_review_result(get_ai_review(diff))
        print("\n===== AI Code Review (Markdown) =====")
        print(review)
        print("===== End AI Code Review =====\n")
        print("Posting review to GitLab commit...")
        post_comment_to_commit(review)
        print(f"AI Review complete: {review_result}")

        if review_result == "CRITICAL":
            print("Critical issue found. Marking this allowed-to-fail job as a warning.")
            sys.exit(1)
    else:
        print("No changes detected in diff. Skipping AI review.")