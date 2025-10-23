@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

rem ==========================================
rem 🧱 Basic configuration
rem ==========================================
set ROOT_DIR=%~dp0..
set BUILD_DIR=%ROOT_DIR%\build
set SOURCE_DIR=%ROOT_DIR%
set GRAPHVIZ_FILE=%BUILD_DIR%\cmake_graph\cmake_graph.dot

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
echo Start generating project on Windows
echo ROOT_DIR   : %ROOT_DIR%
echo SOURCE_DIR : %SOURCE_DIR%
echo BUILD_DIR  : %BUILD_DIR%
echo CMAKE      : %CMAKE%
echo ****************************************************

rem ==========================================
rem 🚀 Run CMake configuration
rem ==========================================
"%CMAKE%" -S "%SOURCE_DIR%" -B "%BUILD_DIR%" ^
    -G "Visual Studio 17 2022" -A x64 ^
    --graphviz "%GRAPHVIZ_FILE%" ^
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="%BUILD_DIR%\bin" ^
    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="%BUILD_DIR%\bin" ^
    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="%BUILD_DIR%\bin" ^
    -DCMAKE_INSTALL_PREFIX="%ROOT_DIR%\install" ^
    -DCMAKE_VERBOSE_MAKEFILE=ON

set EXIT_CODE=%errorlevel%

rem ==========================================
rem 📦 Result summary
rem ==========================================
if %EXIT_CODE% NEQ 0 (
    echo [ERROR] CMake configuration failed with code %EXIT_CODE%.
    exit /b %EXIT_CODE%
) else (
    echo ****************************************************
    echo ✅ CMake project generated successfully.
    echo Build files are located in: "%BUILD_DIR%"
    echo ****************************************************
)

endlocal
exit /b %EXIT_CODE%
