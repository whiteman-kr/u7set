import os
import subprocess
import requests

GITLAB_TOKEN = os.environ.get("GITLAB_API_TOKEN")
CI_PROJECT_ID = os.environ.get("CI_PROJECT_ID")
CI_COMMIT_SHA = os.environ.get("CI_COMMIT_SHA")
CI_BEFORE_SHA = os.environ.get("CI_COMMIT_BEFORE_SHA")
CI_API_V4_URL = os.environ.get("CI_API_V4_URL")
OLLAMA_URL = os.environ.get("OLLAMA_URL", "http://127.0.0.1:11434")
OLLAMA_TIMEOUT = int(os.environ.get("OLLAMA_TIMEOUT", "600"))
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
    # Only review C++ file changes if diff is large, or ask Ollama directly
    prompt = (
        "You are an expert C++ code reviewer enforcing modern C++23 standards. "
        "Review the following code changes. "
        "1. Identify bugs, memory leaks, and undefined behavior. "
        "2. Recommend C++23 features where applicable (e.g., std::expected, std::print, deducing 'this', monadic std::optional) only were it really required. "
        "3. Flag outdated C++ idioms which are obsolete. "
        "Keep the review concise and provide short code examples for your suggestions.\n\n"
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

def post_comment_to_commit(review_text):
    # Posts directly under the Commit in GitLab UI
    url = f"{CI_API_V4_URL}/projects/{CI_PROJECT_ID}/repository/commits/{CI_COMMIT_SHA}/comments"
    headers = {"PRIVATE-TOKEN": GITLAB_TOKEN}
    payload = {"note": f"### 🤖 AI Code Review\n\n{review_text}"}
    
    response = requests.post(url, headers=headers, json=payload)
    response.raise_for_status()

if __name__ == "__main__":
    diff = get_git_diff()
    
    # Filter or check if diff contains actual content
    if diff and diff.strip():
        print("Sending diff to Ollama on host...")
        review = get_ai_review(diff)
        print("Posting review to GitLab commit...")
        post_comment_to_commit(review)
        print("AI Review complete!")
    else:
        print("No changes detected in diff. Skipping AI review.")