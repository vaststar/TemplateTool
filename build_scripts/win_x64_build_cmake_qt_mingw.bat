@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

rem =========================================
rem 🧱 Basic configuration
rem =========================================
set "ROOT_DIR=%~dp0.."
set "BUILD_DIR=%ROOT_DIR%\build_mingw"
set "TARGET_NAME=mainEntry"
set "BUILD_TYPE=Release"

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
    exit /b 1
)

rem =========================================
rem 🧩 Check if build directory exists
rem =========================================
if not exist "%BUILD_DIR%" (
    echo [ERROR] Build directory not found: "%BUILD_DIR%"
    echo Please run win_x64_generate_cmake_qt_mingw.bat first.
    exit /b 1
)

rem =========================================
rem 🔍 Detect CPU cores for parallel build
rem =========================================
set "PARALLEL_JOBS=%NUMBER_OF_PROCESSORS%"
if not defined PARALLEL_JOBS (
    set "PARALLEL_JOBS=4"
)

rem =========================================
rem 📋 Display configuration summary
rem =========================================
echo ****************************************************
echo Start building project on Windows with MinGW
echo ROOT_DIR    : "%ROOT_DIR%"
echo BUILD_DIR   : "%BUILD_DIR%"
echo CMAKE       : "%CMAKE%"
echo TARGET_NAME : %TARGET_NAME%
echo BUILD_TYPE  : %BUILD_TYPE%
echo PARALLEL    : %PARALLEL_JOBS% jobs
echo ****************************************************

rem =========================================
rem 🚀 Run CMake build
rem =========================================
echo [INFO] Building project...

"%CMAKE%" --build "%BUILD_DIR%" ^
    --target %TARGET_NAME% ^
    --config %BUILD_TYPE% ^
    --parallel %PARALLEL_JOBS%

set "EXIT_CODE=%errorlevel%"

rem =========================================
rem 📦 Build result summary
rem =========================================
if not "%EXIT_CODE%"=="0" (
    echo ****************************************************
    echo [ERROR] Build failed with code %EXIT_CODE%.
    echo ****************************************************
    exit /b %EXIT_CODE%
) else (
    echo ****************************************************
    echo ✅ Build succeeded.
    echo Output directory: "%BUILD_DIR%\bin"
    echo.
    echo 📝 Next steps:
    echo   - Install: cmake --install "%BUILD_DIR%"
    echo   - Run: "%BUILD_DIR%\bin\%TARGET_NAME%.exe"
    echo ****************************************************
)

endlocal
exit /b %EXIT_CODE%