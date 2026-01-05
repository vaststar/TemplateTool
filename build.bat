@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

rem ==========================================
rem 🧱 Basic configuration
rem ==========================================
set "PRESET=%~1"
set "ACTION=%~2"

if "%PRESET%"=="" set "PRESET=windows-msvc-release"
if "%ACTION%"=="" set "ACTION=all"

set "ROOT_DIR=%~dp0"
set "BUILD_DIR=%ROOT_DIR%build\%PRESET%"
set "GRAPHVIZ_DIR=%BUILD_DIR%\cmake_graph"
set "GRAPHVIZ_FILE=%GRAPHVIZ_DIR%\cmake_graph.dot"

rem ==========================================
rem 🔍 Detect system CMake
rem ==========================================
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

rem ==========================================
rem 📋 Display configuration summary
rem ==========================================
echo ****************************************************
echo Start CMake Presets workflow
echo ROOT_DIR   : "%ROOT_DIR%"
echo BUILD_DIR  : "%BUILD_DIR%"
echo CMAKE      : "%CMAKE%"
echo PRESET     : %PRESET%
echo ACTION     : %ACTION%
echo ****************************************************

rem ==========================================
rem 🚀 Execute action
rem ==========================================
if /i "%ACTION%"=="configure" goto :configure
if /i "%ACTION%"=="config" goto :configure
if /i "%ACTION%"=="generate" goto :configure
if /i "%ACTION%"=="build" goto :build
if /i "%ACTION%"=="clean" goto :clean
if /i "%ACTION%"=="install" goto :install
if /i "%ACTION%"=="package" goto :package
if /i "%ACTION%"=="all" goto :all
goto :all

:configure
call :ensure_graphviz_dir
echo [INFO] Configuring project...
"%CMAKE%" --preset %PRESET% --graphviz="%GRAPHVIZ_FILE%"
set "EXIT_CODE=%errorlevel%"
goto :result

:build
call :ensure_configured
echo [INFO] Building project...
"%CMAKE%" --build --preset %PRESET%
set "EXIT_CODE=%errorlevel%"
goto :result

:clean
call :is_configured
if %errorlevel%==0 (
    echo [INFO] Cleaning build directory...
    "%CMAKE%" --build --preset %PRESET% --target clean
    set "EXIT_CODE=%errorlevel%"
) else (
    echo [WARNING] Project not configured, nothing to clean.
    set "EXIT_CODE=0"
)
goto :result

:install
call :ensure_configured
echo [INFO] Building and installing...
"%CMAKE%" --build --preset %PRESET%
if %errorlevel%==0 (
    "%CMAKE%" --install "%BUILD_DIR%"
)
set "EXIT_CODE=%errorlevel%"
goto :result

:package
call :ensure_configured
echo [INFO] Building and packaging...
"%CMAKE%" --build --preset %PRESET%
if %errorlevel%==0 (
    cpack --preset %PRESET% -B "%BUILD_DIR%"
    set "EXIT_CODE=%errorlevel%"
) else (
    set "EXIT_CODE=%errorlevel%"
)
goto :result

:all
call :ensure_graphviz_dir
echo [INFO] Configure + Build
"%CMAKE%" --preset %PRESET% --graphviz="%GRAPHVIZ_FILE%"
set "EXIT_CODE=%errorlevel%"
if %EXIT_CODE%==0 (
    "%CMAKE%" --build --preset %PRESET%
    set "EXIT_CODE=%errorlevel%"
)
goto :result

rem ==========================================
rem 🧩 Helper functions
rem ==========================================
:ensure_graphviz_dir
if not exist "%GRAPHVIZ_DIR%" (
    mkdir "%GRAPHVIZ_DIR%" 2>nul
)
exit /b 0

:is_configured
if exist "%BUILD_DIR%\CMakeCache.txt" (
    exit /b 0
) else (
    exit /b 1
)

:ensure_configured
call :is_configured
if %errorlevel% neq 0 (
    echo [INFO] Project not configured yet, configuring first...
    call :ensure_graphviz_dir
    "%CMAKE%" --preset %PRESET% --graphviz="%GRAPHVIZ_FILE%"
    if %errorlevel% neq 0 (
        echo [ERROR] Configuration failed.
        exit /b 1
    )
)
exit /b 0

rem ==========================================
rem 📦 Result summary
rem ==========================================
:result
if not "%EXIT_CODE%"=="0" (
    echo [ERROR] Operation failed with code %EXIT_CODE%.
    exit /b %EXIT_CODE%
) else (
    echo ****************************************************
    echo [SUCCESS] Operation completed successfully.
    echo Build files are located in: "%BUILD_DIR%"
    if /i "%ACTION%"=="package" (
        echo Package files are located in: "%BUILD_DIR%"
    )
    echo ****************************************************
)

endlocal
exit /b 0