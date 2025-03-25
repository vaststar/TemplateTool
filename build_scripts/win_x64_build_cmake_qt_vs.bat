@echo off

set ROOT_DIR=%~dp0..\
set CMAKE=%ROOT_DIR%tools\platforms\windows\x64\cmake\bin\cmake.exe
set BUILD_DIR=%ROOT_DIR%build
set TARGET_NAME=mainEntry
set BUILD_TYPE=Debug

echo ****************************************************
echo Start build project on Windows
echo ROOT_DIR is %ROOT_DIR%
echo ****************************************************

echo =========================================================
echo CMAKE: %CMAKE%
echo BUILD_DIR: "%BUILD_DIR%"
echo TARGET_NAME: "%TARGET_NAME%" 
echo BUILD_TYPE: "%BUILD_TYPE%"
echo =========================================================

"%CMAKE%" --build "%BUILD_DIR%" --target "%TARGET_NAME%" --config "%BUILD_TYPE%"

set EXIT_CODE=%errorlevel%

echo ****************************************************
echo Finish build project on Windows
echo ROOT_DIR is %ROOT_DIR%
echo BUILD_DIR is %BUILD_DIR%
echo ****************************************************

exit /B %EXIT_CODE%
