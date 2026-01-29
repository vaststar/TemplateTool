@echo off
REM Compress large binary files for Git storage
REM Double-click to run, or use: compress_large_files.bat [platform]
REM   platform: all, android, windows (default: all)

cd /d "%~dp0"
powershell -ExecutionPolicy Bypass -File "%~dp0compress_large_files.ps1" -Force %*
pause
