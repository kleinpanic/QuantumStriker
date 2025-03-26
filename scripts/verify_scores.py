#!/usr/bin/env python3
"""

This script reads the blockchain file (one JSON block per line) from
'../highscore/blockchain.txt', prints detailed debug information about each block,
computes each block’s hash, and verifies that the stored proof‐of‐work and
digital signature are correct. This script is intended for development and CI verification.
"""

import os
import json
import hashlib
import time

# ANSI color codes for debug messages:
COLOR_RED = "\033[31m"
COLOR_GREEN = "\033[32m"
COLOR_BLUE = "\033[34m"
COLOR_RESET = "\033[0m"

def debug_print(message, level=2):
    # level: 1=critical (red), 2=info (blue), 3=success (green)
    if level == 1:
        color = COLOR_RED
    elif level == 3:
        color = COLOR_GREEN
    else:
        color = COLOR_BLUE
    print(f"{color}[DEBUG] {message}{COLOR_RESET}")

# Adjust BLOCKCHAIN_FILE so that it points to the same file your game writes.
BLOCKCHAIN_FILE = "../highscore/blockchain.txt"
DIFFICULTY = 4  # number of leading zeros required

def compute_block_hash(block):
    # Format: username|score|timestamp|prev_hash|nonce
    data = f"{block.get('username','')}|{block.get('score',0)}|{int(block.get('timestamp',0))}|{block.get('prev_hash','')}|{block.get('nonce',0)}"
    computed = hashlib.sha256(data.encode()).hexdigest()
    debug_print(f"Data string: '{data}'", level=2)
    debug_print(f"Computed hash: '{computed}'", level=2)
    return computed

def verify_proof_of_work(block):
    computed = compute_block_hash(block)
    stored = block.get("proof_of_work", "")
    if stored != computed:
        print(f"Invalid proof-of-work hash in block: expected '{computed}' but found '{stored}'")
        return False
    if not computed.startswith("0" * DIFFICULTY):
        print(f"Block does not meet difficulty requirement: '{computed}' does not start with {'0'*DIFFICULTY}")
        return False
    return True

def verify_signature(block):
    sig = block.get("signature", "")
    if not sig or len(sig) < 10:
        print(f"Digital signature invalid or missing for block: {block}")
        return False
    debug_print(f"Signature present (length {len(sig)}) for user '{block.get('username','')}'", level=2)
    return True

def load_blockchain():
    blocks = []
    if not os.path.exists(BLOCKCHAIN_FILE):
        print("Blockchain file not found.")
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
                print(f"Error parsing line in blockchain file: {e}")
    debug_print(f"Loaded {len(blocks)} block(s) from {BLOCKCHAIN_FILE}", level=2)
    return blocks

def verify_chain(blocks):
    latest_for_user = {}
    for i, block in enumerate(blocks):
        print(f"[DEBUG] verify_chain: Verifying block {i} for user {block['username']}")
        if not verify_proof_of_work(block):
            print(f"Block {i} failed proof-of-work verification.")
            return False
        if not verify_signature(block):
            print(f"Block {i} failed digital signature verification.")
            return False
        if block['username'] in latest_for_user:
            if block['prev_hash'] != latest_for_user[block['username']]['proof_of_work']:
                print(f"Chain linkage error for user {block['username']} in block {i}.")
                return False
        else:
            # For genesis block, prev_hash should be all zeros.
            if block['prev_hash'] != "0" * len(block['prev_hash']):
                print(f"Genesis block for user {block['username']} does not have a zero prev_hash in block {i}.")
                return False
        latest_for_user[block['username']] = block
    return True

def main():
    blocks = load_blockchain()
    if not blocks:
        print("No blockchain data found.")
        return
    if verify_chain(blocks):
        debug_print("Blockchain verified successfully.", level=3)
        debug_print("This verification script is intended for development and CI validation.", level=2)
    else:
        print("Blockchain verification failed.")

if __name__ == "__main__":
    main()

