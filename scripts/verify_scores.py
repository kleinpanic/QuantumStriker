#!/usr/bin/env python3
"""
verify_scores.py - Debugging blockchain verification script.

This script reads the blockchain file (one JSON block per line) from 
'../highscore/blockchain.txt', prints out all fields of each block,
computes the block hash, and verifies that the stored proof‐of‐work is correct.
It also shows a detailed diff between the expected and computed hash.
"""

import os
import json
import hashlib
import time

# Adjust BLOCKCHAIN_FILE so that it points to the same file your game writes.
BLOCKCHAIN_FILE = "../highscore/blockchain.txt"
DIFFICULTY = 4  # number of leading zeros required

def compute_block_hash(block):
    # Build the data string exactly as done in the C code:
    # Format: username|score|timestamp|prev_hash|nonce
    # Note: Make sure that all fields are exactly as stored!
    data = f"{block.get('username','')}|{block.get('score',0)}|{int(block.get('timestamp',0))}|{block.get('prev_hash','')}|{block.get('nonce',0)}"
    computed = hashlib.sha256(data.encode()).hexdigest()
    print(f"[DEBUG][Py] Data string: '{data}'")
    print(f"[DEBUG][Py] Computed hash: '{computed}'")
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
    # Stub: here we assume that the signature is valid if it exists and is long enough.
    sig = block.get("signature", "")
    if not sig or len(sig) < 10:
        print(f"Digital signature invalid or missing for block: {block}")
        return False
    print(f"[DEBUG][Py] Signature present (length {len(sig)}) for user '{block.get('username','')}'")
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
    print(f"[DEBUG][Py] Loaded {len(blocks)} block(s) from {BLOCKCHAIN_FILE}")
    return blocks

def verify_chain(blocks):
    latest_for_user = {}
    for i, block in enumerate(blocks):
        print(f"[DEBUG][Py] verify_chain: Verifying block {i} for user {block['username']}")
        if not verify_proof_of_work(block):
            print(f"Block {i} failed proof-of-work verification.")
            return False
        if not verify_signature(block):
            print(f"Block {i} failed digital signature verification.")
            return False
        # Only check chain linkage if this is not the genesis block
        if i > 0:
            if block['prev_hash'] != latest_for_user[block['username']]['proof_of_work']:
                print(f"Chain linkage error for user {block['username']} in block {i}.")
                return False
        latest_for_user[block['username']] = block
    return True

def main():
    blocks = load_blockchain()
    if not blocks:
        print("No blockchain data found.")
        return
    if verify_chain(blocks):
        print("\nBlockchain verified successfully.")
    else:
        print("\nBlockchain verification failed.")

if __name__ == "__main__":
    # Define constant for HASH_STR_LEN used in Python for comparison.
    HASH_STR_LEN = 65  # 64 hex digits plus null terminator (not used in Python, but for clarity)
    main()

