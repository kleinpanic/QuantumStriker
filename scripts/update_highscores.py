#!/usr/bin/env python3
import os
import re
import sys
import json
import hashlib
import time

# Configuration
BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
BLOCKCHAIN_FILE = os.path.join(BASE_DIR, "highscore", "blockchain.txt")
README_PATH = os.path.join(BASE_DIR, "README.md")
DIFFICULTY = 4  # Must match the C side

def hash_score(data):
    return hashlib.sha256(data.encode()).hexdigest()

def compute_block_hash(block):
    # Build the data string exactly as done in the C code:
    # Format: username|score|timestamp|prev_hash|nonce
    data = f"{block.get('username','')}|{block.get('score',0)}|{int(block.get('timestamp',0))}|{block.get('prev_hash','')}|{block.get('nonce',0)}"
    computed = hash_score(data)
    # Debug info
    print(f"[DEBUG][Py] Data string: '{data}'")
    print(f"[DEBUG][Py] Computed hash: '{computed}'")
    return computed

def verify_proof_of_work(block):
    computed = compute_block_hash(block)
    stored = block.get("proof_of_work", "")
    if stored != computed:
        print(f"Invalid PoW: expected '{computed}' but found '{stored}'")
        return False
    if not computed.startswith("0" * DIFFICULTY):
        print(f"Difficulty not met: '{computed}' does not start with {'0'*DIFFICULTY}")
        return False
    return True

def verify_signature(block):
    # Stub: Here we assume the signature is valid if it exists and its length is above a threshold.
    sig = block.get("signature", "")
    if not sig or len(sig) < 10:
        print(f"Invalid signature for block: {block}")
        return False
    return True

def load_blockchain():
    blocks = []
    if not os.path.exists(BLOCKCHAIN_FILE):
        print(f"Blockchain file '{BLOCKCHAIN_FILE}' not found.")
        return blocks
    with open(BLOCKCHAIN_FILE, "r") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            try:
                block = json.loads(line)
                blocks.append(block)
            except Exception as e:
                print(f"Error parsing block: {e}")
    print(f"[DEBUG][Py] Loaded {len(blocks)} block(s) from {BLOCKCHAIN_FILE}")
    return blocks

def partition_blocks(blocks):
    valid_blocks = []
    cheater_blocks = []
    for block in blocks:
        if verify_proof_of_work(block) and verify_signature(block):
            valid_blocks.append(block)
        else:
            cheater_blocks.append(block)
    return valid_blocks, cheater_blocks

def get_top_scores(blocks):
    """
    Build a dictionary mapping each username to their highest valid score.
    Returns a list of tuples: (username, score, block), sorted descending by score.
    """
    top_scores = {}
    for block in blocks:
        user = block.get('username', '')
        score = block.get('score', 0)
        if user not in top_scores or score > top_scores[user][1]:
            top_scores[user] = (block, score)
    result = [(user, data[1], data[0]) for user, data in top_scores.items()]
    result.sort(key=lambda x: x[1], reverse=True)
    return result

def generate_markdown_table(top_scores):
    header = "| Rank | Username           | Score | Timestamp |\n"
    header += "|------|--------------------|-------|-----------|\n"
    rows = []
    for i, (username, score, block) in enumerate(top_scores[:3]):
        ts = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(block.get('timestamp',0)))
        rows.append(f"| {i+1}    | {username:<18} | {score:<5} | {ts} |")
    table = header + "\n".join(rows)
    return table

def generate_cheaters_table(cheater_blocks):
    if not cheater_blocks:
        return "No cheaters found."
    header = "| Username           | Score | Timestamp | Reason |\n"
    header += "|--------------------|-------|-----------|--------|\n"
    rows = []
    for block in sorted(cheater_blocks, key=lambda b: b.get("score", 0), reverse=True):
        username = block.get("username", "")
        score = block.get("score", 0)
        ts = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(block.get("timestamp",0)))
        # Determine reason(s) for failure
        reasons = []
        if not verify_proof_of_work(block):
            reasons.append("Invalid PoW")
        if not verify_signature(block):
            reasons.append("Invalid Signature")
        reason_str = ", ".join(reasons)
        rows.append(f"| {username:<18} | {score:<5} | {ts} | {reason_str} |")
    table = header + "\n".join(rows)
    return table

def update_section_in_file(marker_start, marker_end, new_content):
    if not os.path.exists(README_PATH):
        print("README.md not found.")
        sys.exit(1)
    with open(README_PATH, "r") as f:
        content = f.read()
    pattern = re.compile(
        rf"({re.escape(marker_start)})(.*?){re.escape(marker_end)}", re.DOTALL
    )
    updated_section = f"{marker_start}\n{new_content}\n{marker_end}"
    new_file_content, count = pattern.subn(updated_section, content)
    if count == 0:
        print(f"Markers {marker_start} and {marker_end} not found in README.md.")
        sys.exit(1)
    with open(README_PATH, "w") as f:
        f.write(new_file_content)
    return new_file_content

def main():
    print("Starting high score update...")
    all_blocks = load_blockchain()
    if not all_blocks:
        print("No blockchain data found.")
        sys.exit(0)
    
    valid_blocks, cheater_blocks = partition_blocks(all_blocks)
    print(f"[DEBUG][Py] Valid blocks: {len(valid_blocks)}, Cheater blocks: {len(cheater_blocks)}")
    
    top_scores = get_top_scores(valid_blocks)
    if not top_scores:
        print("No top scores found.")
        sys.exit(0)
    
    table = generate_markdown_table(top_scores)
    update_section_in_file("<!-- TOP_SCORES_START -->", "<!-- TOP_SCORES_END -->", table)
    print("Top high scores table updated.")
    
    cheaters_table = generate_cheaters_table(cheater_blocks)
    update_section_in_file("<!-- CHEATERS_START -->", "<!-- CHEATERS_END -->", cheaters_table)
    print("Cheaters table updated.")
    
    if os.getenv("GITHUB_ACTIONS") is None:
        print("High score update complete. Please push your changes to update the repository.")
    else:
        print("High score update complete (running under GitHub Actions).")

if __name__ == "__main__":
    main()

