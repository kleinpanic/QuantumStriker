#!/usr/bin/env python3
import os
import re
import sys

# Compute repository root relative to this script.
BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
HIGH_SCORE_DIR = os.path.join(BASE_DIR, "highscore")
README_PATH = os.path.join(BASE_DIR, "README.md")

def get_scores():
    scores = []
    if not os.path.exists(HIGH_SCORE_DIR):
        print(f"Highscore directory '{HIGH_SCORE_DIR}' not found.")
        return scores
    for filename in os.listdir(HIGH_SCORE_DIR):
        if filename.endswith("_highscore.txt"):
            username = filename[:-len("_highscore.txt")]
            if not username:
                username = "n/a"
            try:
                with open(os.path.join(HIGH_SCORE_DIR, filename), "r") as f:
                    content = f.read().strip()
                    score = int(content)
                    scores.append((username, score))
            except Exception as e:
                print(f"Error reading {filename}: {e}")
    return scores

def generate_markdown_table(top_scores):
    header = "| Rank | Username           | Score | Trophy |\n"
    header += "|------|--------------------|-------|--------|\n"
    rows = []
    # Use GitHub emoji codes for medals.
    trophies = [":first_place_medal:", ":second_place_medal:", ":third_place_medal:"]
    for i, (username, score) in enumerate(top_scores):
        rows.append(f"| {i+1}    | {username:<18} | {score:<5} | {trophies[i]} |")
    table = header + "\n".join(rows)
    return table

def generate_badges(top_scores):
    # Create a badge for each of the top 3 scores on its own line.
    badges_lines = []
    labels = ["1st", "2nd", "3rd"]
    colors = ["gold", "silver", "bronze"]
    # We'll include GitHub username info in the badge text.
    for i, (username, score) in enumerate(top_scores):
        if not username:
            username = "n/a"
        # Build a message of the form "username | score" and URL-encode spaces.
        message = f"{username}|{score}".replace(" ", "%20")
        # Create a badge URL using Shields.io. 
        badge_url = f"https://img.shields.io/badge/{labels[i]}-{message}-{colors[i]}"
        # Each badge is on its own line, with a descriptive alt text including a trophy emoji.
        # Note: The GitHub emoji codes here are literal text; they will render as emoji when processed by GitHub.
        badges_lines.append(f"![{labels[i]} Place {':first_place_medal:' if i==0 else (':second_place_medal:' if i==1 else ':third_place_medal:')}]({badge_url})")
    return "\n".join(badges_lines)

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
    scores = get_scores()
    if not scores:
        print("No scores found. Exiting.")
        sys.exit(0)
    # Sort descending by score.
    scores.sort(key=lambda x: x[1], reverse=True)
    top_scores = scores[:3]
    
    # Generate and update markdown table for top scores.
    table = generate_markdown_table(top_scores)
    update_section_in_file("<!-- TOP_HIGHSCORES_START -->", "<!-- TOP_HIGHSCORES_END -->", table)
    print("Top high scores table updated.")
    
    # Generate and update badges for top 3, one per line.
    badges = generate_badges(top_scores)
    update_section_in_file("<!-- HIGH_SCORE_BADGE_START -->", "<!-- HIGH_SCORE_BADGE_END -->", badges)
    print("High score badges updated.")
    
    # If not running in GitHub Actions, provide extra debug output.
    if os.getenv("GITHUB_ACTIONS") is None:
        print("High score update complete.")
        print("Please push your changes or submit a pull request to update the repository.")
    else:
        print("High score update complete (running under GitHub Actions).")
    
if __name__ == "__main__":
    main()

