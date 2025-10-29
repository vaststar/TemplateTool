@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

rem =========================================
rem 🧱 Basic configuration
rem =========================================
set "ROOT_DIR=%~dp0.."
set "BUILD_DIR=%ROOT_DIR%\build"
set "SOURCE_DIR=%ROOT_DIR%"
set "GRAPHVIZ_FILE=%BUILD_DIR%\cmake_graph.dot"
set "INSTALL_DIR=%ROOT_DIR%\install"

rem =========================================
rem 🔍 Detect system CMake
rem =========================================
set "CMAKE="
where cmake >nul 2>nul
if %errorlevel%==0 (
    for /f "delims=" %%i in ('where cmake') do (
        set "CMAKE=%%i"
        goto :found_cmake
    )
)
:found_cmake
if not defined CMAKE (
    echo [ERROR] System CMake not found.
    echo Please install CMake and ensure it is in your PATH.
    echo Download: https://cmake.org/download/
    exit /b 1
)

rem =========================================
rem 🧩 Ensure build directory exists
rem =========================================
if not exist "%BUILD_DIR%" (
    echo [INFO] Build directory not found, creating...
    mkdir "%BUILD_DIR%" >nul 2>nul
    if %errorlevel% neq 0 (
        echo [ERROR] Failed to create build directory: "%BUILD_DIR%"
        exit /b 1
    )
)

rem =========================================
rem 🔍 Detect Visual Studio generator automatically
rem =========================================
rem Call external script detect_vs_generator.bat to get the correct
rem CMake generator string, e.g. "Visual Studio 17 2022 -A x64"
rem
rem The script must be in the same directory as this batch file,
rem or adjust the path accordingly below.
rem
rem Output: Just the generator string, e.g.:
rem         Visual Studio 17 2022 -A x64
rem =========================================
set "DETECT_SCRIPT=%~dp0detect_vs_generator.bat"
rem --------------------------
rem Call the detection script ONCE, capture both output and exit code
rem --------------------------

rem Redirect the script output to a temporary file
set "TEMP_OUTPUT=%TEMP%\vs_generator_output.tmp"

call "%DETECT_SCRIPT%" > "%TEMP_OUTPUT%" 2>&1
set "DETECT_EXIT_CODE=%errorlevel%"

rem --------------------------
rem Check exit code: if non-zero, detection failed
rem --------------------------
if %DETECT_EXIT_CODE% neq 0 (
    echo [ERROR] Visual Studio generator detection failed with exit code %DETECT_EXIT_CODE%.
    echo Check the detection script for details.
    del "%TEMP_OUTPUT%" >nul 2>&1
    exit /b %DETECT_EXIT_CODE%
)

rem --------------------------
rem Read the output content from the temp file as the VS generator
rem --------------------------
set /p VS_GENERATOR=<"%TEMP_OUTPUT%"

rem --------------------------
rem Clean up the temporary file
rem --------------------------
del "%TEMP_OUTPUT%" >nul 2>&1

rem --------------------------
rem Optional: check if the captured content is empty
rem --------------------------
if "!VS_GENERATOR!"=="" (
    echo [ERROR] Detected Visual Studio generator is empty.
    exit /b 1
)

rem --------------------------
rem Normal output of the detected generator
rem --------------------------
echo [INFO] Detected Visual Studio generator: !VS_GENERATOR!

rem --------------------------
rem Assume %VS_GENERATOR% = "Visual Studio 17 2022 -A x64"
rem We want to split into:
rem   - Generator name: Visual Studio 17 2022
rem   - Architecture argument: -A x64
rem --------------------------

rem ✅ Remove the trailing " -A x64" to get the generator name
set "GENERATOR_NAME=!VS_GENERATOR: -A x64=!"

rem ✅ The architecture argument is fixed
set "ARCHITECTURE=-A x64"

rem ✅ Optional: print debug info
echo [DEBUG] Generator Name: !GENERATOR_NAME!
echo [DEBUG] Architecture: !ARCHITECTURE!

rem =========================================
rem 📋 Display configuration summary
rem =========================================
echo ****************************************************
echo Start generating project on Windows
echo ROOT_DIR   : "%ROOT_DIR%"
echo SOURCE_DIR : "%SOURCE_DIR%"
echo BUILD_DIR  : "%BUILD_DIR%"
echo CMAKE      : "%CMAKE%"
echo CMake Generator: %VS_GENERATOR%
echo ****************************************************

rem =========================================
rem 🚀 Run CMake configuration with detected generator
rem =========================================
"%CMAKE%" -S "%SOURCE_DIR%" -B "%BUILD_DIR%" ^
    -G "!GENERATOR_NAME!" !ARCHITECTURE! ^
    --graphviz "%GRAPHVIZ_FILE%" ^
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="%BUILD_DIR%\bin" ^
    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="%BUILD_DIR%\bin" ^
    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="%BUILD_DIR%\bin" ^
    -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" ^
    -DCMAKE_VERBOSE_MAKEFILE=ON

set "EXIT_CODE=%errorlevel%"

rem =========================================
rem 📦 Result summary
rem =========================================
if not "%EXIT_CODE%"=="0" (
    echo [ERROR] CMake configuration failed with code %EXIT_CODE%.
    exit /b %EXIT_CODE%
) else (
    echo ****************************************************
    echo ✅ CMake project generated successfully.
    echo Build files are located in: "%BUILD_DIR%"
    echo ****************************************************
)

endlocal
exit /b 0