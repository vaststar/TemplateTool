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
    if errorlevel 1 exit /b %errorlevel%
)

set VENV_PYTHON=%SCRIPT_DIR%build_venv\Scripts\python.exe

if not exist "%VENV_PYTHON%" (
    echo Failed to locate virtual environment Python: %VENV_PYTHON%
    exit /b 1
)

REM Install dependencies
"%VENV_PYTHON%" -m pip install --upgrade pip
if errorlevel 1 exit /b %errorlevel%

"%VENV_PYTHON%" -m pip install -r requirements.txt
if errorlevel 1 exit /b %errorlevel%

REM Build with PyInstaller
"%VENV_PYTHON%" -m PyInstaller ^
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
if errorlevel 1 exit /b %errorlevel%

echo.
echo === Build complete ===
echo Output: %SCRIPT_DIR%dist\proxy_addon\proxy_addon.exe
