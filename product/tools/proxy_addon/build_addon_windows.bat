@echo off
REM Build the proxy addon as a standalone binary using PyInstaller (Windows)
REM
REM Prerequisites:
REM   pip install mitmproxy pyinstaller
REM
REM Output:
REM   dist\proxy_addon\proxy_addon.exe

setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
cd /d %SCRIPT_DIR%

echo === Building proxy addon for Windows ===

REM Create virtual environment if not exists
if not exist "build_venv" (
    python -m venv build_venv
)

call build_venv\Scripts\activate.bat

REM Install dependencies
pip install --upgrade pip
pip install -r requirements.txt

REM Build with PyInstaller
pyinstaller ^
    --noconfirm ^
    --clean ^
    --name proxy_addon ^
    --distpath dist ^
    --workpath build_tmp ^
    --specpath build_tmp ^
    --hidden-import mitmproxy ^
    --hidden-import mitmproxy.addons ^
    --hidden-import mitmproxy.tools ^
    --hidden-import mitmproxy.tools.main ^
    --collect-all mitmproxy ^
    --collect-all mitmproxy_rs ^
    proxy_addon.py

call deactivate

echo.
echo === Build complete ===
echo Output: %SCRIPT_DIR%dist\proxy_addon\proxy_addon.exe
