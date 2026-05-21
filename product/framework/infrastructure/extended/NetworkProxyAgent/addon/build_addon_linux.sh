#!/bin/bash
# Build the proxy addon as a standalone binary using PyInstaller (Linux)
#
# Prerequisites:
#   pip3 install mitmproxy pyinstaller
#
# Output:
#   dist/proxy_addon/proxy_addon   (standalone executable)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

echo "=== Building proxy addon for Linux ==="

# Create virtual environment if not exists
if [ ! -d "build_venv" ]; then
    python3 -m venv build_venv
fi

source build_venv/bin/activate

# Install dependencies
pip install --upgrade pip
pip install -r requirements.txt

# Build with PyInstaller
pyinstaller \
    --noconfirm \
    --clean \
    --name proxy_addon \
    --distpath dist \
    --workpath build_tmp \
    --specpath build_tmp \
    --hidden-import mitmproxy \
    --hidden-import mitmproxy.addons \
    --hidden-import mitmproxy.tools \
    --hidden-import mitmproxy.tools.main \
    --collect-all mitmproxy \
    --collect-all mitmproxy_rs \
    proxy_addon.py

deactivate

echo ""
echo "=== Build complete ==="
echo "Output: $SCRIPT_DIR/dist/proxy_addon/proxy_addon"
echo "Size: $(du -sh dist/proxy_addon | cut -f1)"
