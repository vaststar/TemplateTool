@echo off
REM Extract large binary files from .zip archives
REM Double-click to run, or use: extract_large_files.bat [platform]
REM   platform: all, android, windows (default: all)

cd /d "%~dp0"
powershell -ExecutionPolicy Bypass -File "%~dp0extract_large_files.ps1" %*
pause
