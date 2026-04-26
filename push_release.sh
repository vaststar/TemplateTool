#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INFO_JSON="$SCRIPT_DIR/product/framework/version/product_info.json"

if [[ ! -f "$INFO_JSON" ]]; then
    echo "ERROR: $INFO_JSON not found" >&2
    exit 1
fi

# Parse version from product_info.json
MAJOR=$(python3 -c "import json; print(json.load(open('$INFO_JSON'))['version']['major'])")
MINOR=$(python3 -c "import json; print(json.load(open('$INFO_JSON'))['version']['minor'])")
PATCH=$(python3 -c "import json; print(json.load(open('$INFO_JSON'))['version']['patch'])")
TAG="v${MAJOR}.${MINOR}.${PATCH}"

echo "=== Release Tag: $TAG ==="
echo "Creating tag $TAG on HEAD ..."
git tag "$TAG"
git push origin "$TAG"

echo ""
echo "Done. Tag $TAG pushed. GitHub Actions will create the release."
echo "https://github.com/vaststar/TemplateTool/actions"
