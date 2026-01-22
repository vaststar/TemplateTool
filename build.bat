@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

rem ==========================================
rem Basic configuration
rem ==========================================
set "PRESET=%~1"
set "ACTION=%~2"

if "%PRESET%"=="" set "PRESET=windows-msvc-release"
if "%ACTION%"=="" set "ACTION=all"

rem Handle help request
if /i "%PRESET%"=="help" goto :help
if /i "%PRESET%"=="--help" goto :help
if /i "%PRESET%"=="-h" goto :help
if /i "%PRESET%"=="/?" goto :help

set "ROOT_DIR=%~dp0"
set "BUILD_DIR=%ROOT_DIR%build\%PRESET%"
set "GRAPHVIZ_DIR=%BUILD_DIR%\cmake_graph"
set "GRAPHVIZ_FILE=%GRAPHVIZ_DIR%\cmake_graph.dot"

rem ==========================================
rem Detect system CMake
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
rem Display configuration summary
rem ==========================================
for /f "tokens=1-4 delims=/:. " %%a in ("%date% %time%") do (
    set "TIMESTAMP=%%a-%%b-%%c %%d"
)
echo.
echo ****************************************************
echo  CMake Presets Build System
echo ****************************************************
echo  Start Time : %date% %time%
echo  Root Dir   : %ROOT_DIR%
echo  Build Dir  : %BUILD_DIR%
echo  CMake      : %CMAKE%
echo  Preset     : %PRESET%
echo  Action     : %ACTION%
echo ****************************************************
echo.

rem ==========================================
rem Execute action
rem ==========================================
if /i "%ACTION%"=="configure" goto :configure
if /i "%ACTION%"=="config" goto :configure
if /i "%ACTION%"=="generate" goto :configure
if /i "%ACTION%"=="build" goto :build
if /i "%ACTION%"=="rebuild" goto :rebuild
if /i "%ACTION%"=="clean" goto :clean
if /i "%ACTION%"=="install" goto :install
if /i "%ACTION%"=="package" goto :package
if /i "%ACTION%"=="test" goto :test
if /i "%ACTION%"=="all" goto :all

echo [WARNING] Unknown action: %ACTION%
echo [INFO] Valid actions: configure, build, rebuild, clean, install, package, test, all
echo [INFO] Falling back to 'all'
goto :all

:configure
echo.
echo [Step 1/1] Configuring project...
echo ----------------------------------------------------
call :ensure_graphviz_dir
echo   Preset    : %PRESET%
echo   Build Dir : %BUILD_DIR%
echo   Graphviz  : %GRAPHVIZ_FILE%
echo.
"%CMAKE%" --preset %PRESET% --graphviz="%GRAPHVIZ_FILE%"
set "EXIT_CODE=%errorlevel%"
goto :result

:build
echo.
echo [Step 1/1] Building project...
echo ----------------------------------------------------
call :ensure_configured
echo   Preset    : %PRESET%
echo   Build Dir : %BUILD_DIR%
echo.
"%CMAKE%" --build --preset %PRESET%
set "EXIT_CODE=%errorlevel%"
goto :result

:clean
echo.
echo [Step 1/1] Cleaning build artifacts...
echo ----------------------------------------------------
call :is_configured
if !errorlevel!==0 (
    echo   Build Dir : %BUILD_DIR%
    echo.
    "%CMAKE%" --build --preset %PRESET% --target clean
    set "EXIT_CODE=!errorlevel!"
) else (
    echo   [WARNING] Project not configured, nothing to clean.
    set "EXIT_CODE=0"
)
goto :result

:install
echo.
echo [Step 1/1] Building and installing...
echo ----------------------------------------------------
call :ensure_configured
echo   Preset     : %PRESET%
echo   Build Dir  : %BUILD_DIR%
echo   Install to : %ROOT_DIR%install\%PRESET%
echo.
"%CMAKE%" --build --preset %PRESET% --target install
set "EXIT_CODE=%errorlevel%"
goto :result

:package
echo.
echo [Step 1/2] Building project...
echo ----------------------------------------------------
call :ensure_configured
echo   Preset    : %PRESET%
echo   Build Dir : %BUILD_DIR%
echo.
"%CMAKE%" --build --preset %PRESET%
if !errorlevel!==0 (
    echo.
    echo [Step 2/2] Creating package...
    echo ----------------------------------------------------
    cpack --preset %PRESET% -B "%BUILD_DIR%"
    set "EXIT_CODE=!errorlevel!"
) else (
    set "EXIT_CODE=!errorlevel!"
)
goto :result

:rebuild
echo.
echo [Step 1/3] Cleaning previous build...
echo ----------------------------------------------------
call :is_configured
if !errorlevel!==0 (
    echo   Cleaning %BUILD_DIR%
    "%CMAKE%" --build --preset %PRESET% --target clean
) else (
    echo   No previous build found, skipping clean.
)
echo.
echo [Step 2/3] Configuring project...
echo ----------------------------------------------------
call :ensure_graphviz_dir
"%CMAKE%" --preset %PRESET% --graphviz="%GRAPHVIZ_FILE%"
if !errorlevel!==0 (
    echo.
    echo [Step 3/3] Building project...
    echo ----------------------------------------------------
    "%CMAKE%" --build --preset %PRESET%
    set "EXIT_CODE=!errorlevel!"
) else (
    set "EXIT_CODE=!errorlevel!"
)
goto :result

:test
echo.
echo [Step 1/2] Building project...
echo ----------------------------------------------------
call :ensure_configured
echo   Preset    : %PRESET%
echo.
"%CMAKE%" --build --preset %PRESET%
if !errorlevel!==0 (
    echo.
    echo [Step 2/2] Running tests...
    echo ----------------------------------------------------
    ctest --preset %PRESET%
    set "EXIT_CODE=!errorlevel!"
) else (
    set "EXIT_CODE=!errorlevel!"
)
goto :result

:all
echo.
echo [Step 1/2] Configuring project...
echo ----------------------------------------------------
call :ensure_graphviz_dir
echo   Preset    : %PRESET%
echo   Build Dir : %BUILD_DIR%
echo.
"%CMAKE%" --preset %PRESET% --graphviz="%GRAPHVIZ_FILE%"
set "EXIT_CODE=!errorlevel!"
if !EXIT_CODE!==0 (
    echo.
    echo [Step 2/2] Building project...
    echo ----------------------------------------------------
    "%CMAKE%" --build --preset %PRESET%
    set "EXIT_CODE=!errorlevel!"
)
goto :result

rem ==========================================
rem Helper functions
rem ==========================================
:help
echo.
echo Usage: build.bat [PRESET] [ACTION]
echo.
echo PRESET:
echo   windows-msvc-debug      Windows MSVC Debug
echo   windows-msvc-release    Windows MSVC Release (default)
echo   windows-mingw-debug     Windows MinGW Debug
echo   windows-mingw-release   Windows MinGW Release
echo   macos-debug             macOS Debug
echo   macos-release           macOS Release
echo   linux-debug             Linux Debug
echo   linux-release           Linux Release
echo.
echo ACTION:
echo   configure    Configure the project (generate build files)
echo   build        Build the project
echo   rebuild      Clean and rebuild the project
echo   clean        Clean build artifacts
echo   install      Build and install the project
echo   package      Build and create package
echo   test         Build and run tests
echo   all          Configure and build (default)
echo.
echo Examples:
echo   build.bat                                  # Default: windows-msvc-release all
echo   build.bat windows-msvc-debug build         # Debug build only
echo   build.bat windows-msvc-release install     # Release build and install
echo   build.bat windows-msvc-release test        # Run tests
echo.
exit /b 0

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
if !errorlevel! neq 0 (
    echo.
    echo [Step 0] Project not configured, configuring first...
    echo ----------------------------------------------------
    call :ensure_graphviz_dir
    echo   Preset    : %PRESET%
    echo   Build Dir : %BUILD_DIR%
    echo.
    "%CMAKE%" --preset %PRESET% --graphviz="%GRAPHVIZ_FILE%"
    if !errorlevel! neq 0 (
        echo.
        echo [ERROR] Configuration failed, cannot proceed.
        exit /b 1
    )
    echo.
)
exit /b 0

rem ==========================================
rem Result summary
rem ==========================================
:result
echo.
echo ****************************************************
if not "%EXIT_CODE%"=="0" (
    echo  [FAILED] Operation failed with exit code %EXIT_CODE%
    echo ****************************************************
    exit /b %EXIT_CODE%
) else (
    echo  [SUCCESS] Operation completed successfully
    echo ****************************************************
    echo  End Time  : %date% %time%
    echo  Preset    : %PRESET%
    echo  Action    : %ACTION%
    echo  Build Dir : %BUILD_DIR%
    if /i "%ACTION%"=="install" (
        echo  Installed : %ROOT_DIR%install\%PRESET%
    )
    if /i "%ACTION%"=="package" (
        echo  Package   : %BUILD_DIR%
    )
    echo ****************************************************
)

endlocal
exit /b 0