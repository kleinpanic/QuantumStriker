#!/usr/bin/env python3
import os
import re
import sys

def extract_version_from_version_h(path):
    with open(path, 'r') as f:
        content = f.read()
    match = re.search(r'#define\s+QUANTUM_STRIKER_VERSION\s+"([^"]+)"', content)
    if match:
        return match.group(1)
    print("Error: Version not found in version.h")
    sys.exit(1)

def extract_version_from_makefile(path):
    with open(path, 'r') as f:
        for line in f:
            # Assuming the Makefile has a line like: VERSION = x.y.z
            m = re.match(r'^\s*VERSION\s*[:=]\s*([^\s]+)', line)
            if m:
                return m.group(1)
    print("Error: Version not found in Makefile")
    sys.exit(1)

def extract_last_version_from_changelog(path):
    with open(path, 'r') as f:
        lines = f.readlines()
    # Reverse iterate over lines to find a header matching "## [x.y.z]"
    for line in reversed(lines):
        line = line.strip()
        match = re.match(r'^##\s*\[([^]]+)\]', line)
        if match:
            return match.group(1)
    print("Error: Version not found in CHANGELOG.md")
    sys.exit(1)

def main():
    base_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    version_h_path = os.path.join(base_dir, "src", "version.h")
    makefile_path = os.path.join(base_dir, "Makefile")
    changelog_path = os.path.join(base_dir, "CHANGELOG.md")

    version_h = extract_version_from_version_h(version_h_path)
    makefile_version = extract_version_from_makefile(makefile_path)
    changelog_version = extract_last_version_from_changelog(changelog_path)

    print(f"version.h:      {version_h}")
    print(f"Makefile:       {makefile_version}")
    print(f"Changelog:      {changelog_version}")

    if version_h != makefile_version or version_h != changelog_version:
        print("Error: Version mismatch detected!")
        sys.exit(1)
    else:
        print("Success: All versions match.")

if __name__ == "__main__":
    main()

