@echo off

echo =========================================================

set ROOT_DIR=%~dp0..\

set CMAKE=%ROOT_DIR%tools\platforms\windows\x64\cmake\bin\cmake.exe
set BUILD_DIR=%ROOT_DIR%build
set TARGET_NAME=mainEntry
set BUILD_TYPE=Debug

echo CMAKE: %CMAKE%
echo BUILD_DIR: "%BUILD_DIR%"
echo TARGET_NAME: "%TARGET_NAME%" 
echo BUILD_TYPE: "%BUILD_TYPE%"
echo =========================================================

"%CMAKE%" --build "%BUILD_DIR%" --target "%TARGET_NAME%" --config "%BUILD_TYPE%"


set EXIT_CODE=%errorlevel%


exit /B %EXIT_CODE%