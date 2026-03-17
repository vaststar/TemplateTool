#!/bin/bash
# =============================================================================
# Compress Large Files Script
# =============================================================================
# Compresses large binary files to .zip archives for Git storage.
# Use this script when updating the original binary files.
#
# Usage:
#   ./compress_large_files.sh              # Compress all platforms
#   ./compress_large_files.sh android      # Compress Android only
#   ./compress_large_files.sh windows      # Compress Windows only
#   ./compress_large_files.sh macos        # Compress macOS only
#   ./compress_large_files.sh linux        # Compress Linux only
#   ./compress_large_files.sh ios          # Compress iOS only
#   ./compress_large_files.sh all -f       # Force overwrite existing zips
# =============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BASE_DIR="$(dirname "$SCRIPT_DIR")"
CONFIG_PATH="$BASE_DIR/large_files.json"
PLATFORM="${1:-all}"
FORCE=false

# Check for -f or --force flag
for arg in "$@"; do
    if [ "$arg" == "-f" ] || [ "$arg" == "--force" ]; then
        FORCE=true
    fi
done

if [ ! -f "$CONFIG_PATH" ]; then
    echo "Error: Config file not found: $CONFIG_PATH"
    exit 1
fi

echo "=== Compressing Large Files ==="
echo "Platform: $PLATFORM"
echo "Force: $FORCE"
echo ""

COMPRESSED=0
SKIPPED=0
TOTAL_ORIGINAL=0
TOTAL_COMPRESSED=0

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

    if [ ! -f "$ORIGINAL_PATH" ]; then
        echo "[SKIP] Original not found: $FILE_PATH"
        continue
    fi

    if [ -f "$ZIP_PATH" ] && [ "$FORCE" != "true" ]; then
        echo "[SKIP] Zip exists (use -f to overwrite): $FILE_PATH.zip"
        continue
    fi

    echo "[COMPRESS] $FILE_PATH"
    DEST_DIR=$(dirname "$ORIGINAL_PATH")
    FILENAME=$(basename "$ORIGINAL_PATH")

    # Create zip archive
    cd "$DEST_DIR"
    zip -j "$ZIP_PATH" "$FILENAME"
    cd - > /dev/null

    # Show compression stats
    ORIGINAL_SIZE=$(stat -f%z "$ORIGINAL_PATH" 2>/dev/null || stat -c%s "$ORIGINAL_PATH")
    COMPRESSED_SIZE=$(stat -f%z "$ZIP_PATH" 2>/dev/null || stat -c%s "$ZIP_PATH")
    ORIGINAL_MB=$(echo "scale=2; $ORIGINAL_SIZE / 1048576" | bc)
    COMPRESSED_MB=$(echo "scale=2; $COMPRESSED_SIZE / 1048576" | bc)
    RATIO=$(echo "scale=1; (1 - $COMPRESSED_SIZE / $ORIGINAL_SIZE) * 100" | bc)

    echo "  ${ORIGINAL_MB}MB -> ${COMPRESSED_MB}MB (saved ${RATIO}%)"
done

echo ""
echo "Done."
