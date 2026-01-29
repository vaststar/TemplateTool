#!/bin/bash
# =============================================================================
# Extract Large Files Script
# =============================================================================
# Extracts large binary files from .zip archives.
# These files are stored compressed to fit GitHub's 100MB file size limit.
#
# Usage:
#   ./extract_large_files.sh              # Extract all platforms
#   ./extract_large_files.sh android      # Extract Android only
#   ./extract_large_files.sh windows      # Extract Windows only
# =============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BASE_DIR="$(dirname "$SCRIPT_DIR")"
CONFIG_PATH="$BASE_DIR/large_files.json"
PLATFORM="${1:-all}"

if [ ! -f "$CONFIG_PATH" ]; then
    echo "Error: Config file not found: $CONFIG_PATH"
    exit 1
fi

echo "=== Extracting Large Files ==="
echo "Platform: $PLATFORM"
echo ""

EXTRACTED=0
SKIPPED=0

# Parse JSON and process files
python3 -c "
import json
import sys

with open('$CONFIG_PATH') as f:
    config = json.load(f)

for file in config['files']:
    platform = file['platform']
    if '$PLATFORM' != 'all' and platform != '$PLATFORM':
        continue
    print(file['path'] + '|' + platform)
" | while IFS='|' read -r FILE_PATH FILE_PLATFORM; do
    ORIGINAL_PATH="$BASE_DIR/$FILE_PATH"
    ZIP_PATH="$ORIGINAL_PATH.zip"
    
    if [ ! -f "$ZIP_PATH" ]; then
        echo "[SKIP] Zip not found: $FILE_PATH.zip"
        continue
    fi
    
    if [ -f "$ORIGINAL_PATH" ]; then
        echo "[SKIP] Already exists: $FILE_PATH"
        continue
    fi
    
    echo "[EXTRACT] $FILE_PATH"
    DEST_DIR=$(dirname "$ORIGINAL_PATH")
    unzip -o "$ZIP_PATH" -d "$DEST_DIR"
done

echo ""
echo "Done."
