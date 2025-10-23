@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

rem ==========================================
rem 🧱 Basic configuration
rem ==========================================
set ROOT_DIR=%~dp0..
set BUILD_DIR=%ROOT_DIR%\build
set TARGET_NAME=mainEntry
set BUILD_TYPE=Debug

rem ==========================================
rem 🔍 Detect system CMake first (fallback to bundled one)
rem ==========================================
where cmake >nul 2>nul
if %errorlevel%==0 (
    for /f "delims=" %%i in ('where cmake') do set "CMAKE=%%i"
) else (
    echo [WARN] System CMake not found, using bundled version.
    set "CMAKE=%ROOT_DIR%\tools\platforms\windows\x64\cmake\bin\cmake.exe"
)

rem ==========================================
rem 🧩 Ensure build directory exists
rem ==========================================
if not exist "%BUILD_DIR%" (
    echo [INFO] Build directory not found, creating...
    mkdir "%BUILD_DIR%"
)

rem ==========================================
rem 📋 Display configuration summary
rem ==========================================
echo ****************************************************
echo Start building project on Windows
echo ROOT_DIR    : %ROOT_DIR%
echo BUILD_DIR   : %BUILD_DIR%
echo CMAKE       : %CMAKE%
echo TARGET_NAME : %TARGET_NAME%
echo BUILD_TYPE  : %BUILD_TYPE%
echo ****************************************************

rem ==========================================
rem 🚀 Run CMake build
rem ==========================================
"%CMAKE%" --build "%BUILD_DIR%" ^
    --target "%TARGET_NAME%" ^
    --config "%BUILD_TYPE%" ^
    --parallel %NUMBER_OF_PROCESSORS%

set EXIT_CODE=%errorlevel%

rem ==========================================
rem 📦 Build result summary
rem ==========================================
if %EXIT_CODE% NEQ 0 (
    echo [ERROR] Build failed with code %EXIT_CODE%.
    exit /b %EXIT_CODE%
) else (
    echo ****************************************************
    echo ✅ Build succeeded.
    echo Output directory: "%BUILD_DIR%\%BUILD_TYPE%"
    echo ****************************************************
)

endlocal
exit /b %EXIT_CODE%
