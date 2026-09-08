#!/usr/bin/env python3
import os
import glob

BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ASCII_DIR = os.path.join(BASE_DIR, "static", "ascii-art")
OUT_FILE = os.path.join(BASE_DIR, "static", "ascii-art-sizes.css")

# Map filenames to CSS selectors
SELECTORS = {
    "portrait-for-background-ascii-art.txt": "#bg-portrait",
    "work.txt": "#bg-work",
    "my-interests.txt": "#bg-my-interests",
}

css_lines = []

# Static ASCII art files
for fname in sorted(os.listdir(ASCII_DIR)):
    if not fname.endswith(".txt"):
        continue
    fpath = os.path.join(ASCII_DIR, fname)
    with open(fpath, "r", encoding="utf-8", errors="replace") as f:
        text = f.read()
    lines = text.split("\n")
    line_count = len(lines)
    max_chars = max((len(line) for line in lines), default=0)

    if fname in SELECTORS:
        selector = SELECTORS[fname]
    elif fname.startswith("play-") and fname.endswith(".txt"):
        n = fname[5:-4]  # "play-0.txt" -> "0"
        selector = f"#bg-play-{n}"
    else:
        continue

    width_div = max(1, max_chars // 2)
    css_lines.append(f"{selector} {{ --bg-lines: {line_count}; --bg-width-div: {width_div}; }}")

with open(OUT_FILE, "w", encoding="utf-8") as f:
    f.write("\n".join(css_lines) + "\n")

print(f"Generated {OUT_FILE} with {len(css_lines)} rules.")
