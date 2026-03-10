#!/usr/bin/env python3
"""
Normalize platform-specific codegen paths in Qt .ts translation files.

This ensures that .ts files don't show git diff when switching between
macOS (local-macos-release) and Windows (win-x64-release) builds.

Replaces patterns like:
  build/local-macos-release/codegen/...
  build/win-x64-release/codegen/...
With:
  build/CODEGEN/...
"""

import sys
import re
from pathlib import Path


def normalize_ts_file(ts_path: Path) -> bool:
    """
    Normalize codegen paths in a single .ts file.
    Returns True if file was modified.
    """
    content = ts_path.read_text(encoding='utf-8')
    
    # Pattern matches: build/<any-preset>/codegen
    # Examples:
    #   ../../../../../build/local-macos-release/codegen/UIResourceString/...
    #   ../../../../../build/win-x64-release/codegen/UIResourceString/...
    # Replace with: build/CODEGEN/UIResourceString/...
    pattern = r'build/[^/]+/codegen/'
    replacement = r'build/CODEGEN/'
    
    new_content = re.sub(pattern, replacement, content)
    
    if new_content != content:
        ts_path.write_text(new_content, encoding='utf-8')
        print(f"[normalize_ts] Normalized: {ts_path}")
        return True
    else:
        print(f"[normalize_ts] No changes: {ts_path}")
        return False


def main():
    if len(sys.argv) < 2:
        print("Usage: normalize_ts_paths.py <file1.ts> [file2.ts ...]")
        sys.exit(1)
    
    modified_count = 0
    for ts_file in sys.argv[1:]:
        ts_path = Path(ts_file)
        if ts_path.exists() and ts_path.suffix == '.ts':
            if normalize_ts_file(ts_path):
                modified_count += 1
        else:
            print(f"[normalize_ts] Skipped (not found or not .ts): {ts_file}")
    
    print(f"[normalize_ts] Done. {modified_count} file(s) modified.")


if __name__ == '__main__':
    main()
