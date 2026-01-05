@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

rem =========================================
rem 🧱 Basic configuration
rem =========================================
set "ROOT_DIR=%~dp0.."
set "BUILD_DIR=%ROOT_DIR%\build_mingw"
set "SOURCE_DIR=%ROOT_DIR%"
set "GRAPHVIZ_DIR=%BUILD_DIR%\cmake_graph"
set "GRAPHVIZ_FILE=%GRAPHVIZ_DIR%\cmake_graph.dot"
set "INSTALL_DIR=%ROOT_DIR%\install_mingw"

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
rem 🔍 Detect MinGW (gcc and g++)
rem =========================================
set "MINGW_GCC="
set "MINGW_GXX="

where gcc >nul 2>nul
if %errorlevel%==0 (
    for /f "delims=" %%i in ('where gcc') do (
        set "MINGW_GCC=%%i"
        rem Convert backslashes to forward slashes for CMake
        set "MINGW_GCC_CMAKE=!MINGW_GCC:\=/!"
        goto :found_gcc
    )
)
:found_gcc

where g++ >nul 2>nul
if %errorlevel%==0 (
    for /f "delims=" %%i in ('where g++') do (
        set "MINGW_GXX=%%i"
        rem Convert backslashes to forward slashes for CMake
        set "MINGW_GXX_CMAKE=!MINGW_GXX:\=/!"
        goto :found_gxx
    )
)
:found_gxx

if not defined MINGW_GCC (
    echo [ERROR] GCC not found in PATH.
    echo Please install MinGW-w64 or add it to your PATH.
    echo Download: https://www.mingw-w64.org/
    echo Or via MSYS2: pacman -S mingw-w64-x86_64-gcc
    exit /b 1
)

if not defined MINGW_GXX (
    echo [ERROR] G++ not found in PATH.
    echo Please install MinGW-w64 or add it to your PATH.
    exit /b 1
)

rem =========================================
rem 🔍 Detect make/ninja
rem =========================================
set "GENERATOR=MinGW Makefiles"
set "MAKE_TOOL="
set "MAKE_TOOL_CMAKE="

where mingw32-make >nul 2>nul
if %errorlevel%==0 (
    for /f "delims=" %%i in ('where mingw32-make') do (
        set "MAKE_TOOL=%%i"
        rem Convert backslashes to forward slashes for CMake
        set "MAKE_TOOL_CMAKE=!MAKE_TOOL:\=/!"
        set "GENERATOR=MinGW Makefiles"
        goto :found_make
    )
)

where ninja >nul 2>nul
if %errorlevel%==0 (
    for /f "delims=" %%i in ('where ninja') do (
        set "MAKE_TOOL=%%i"
        rem Convert backslashes to forward slashes for CMake
        set "MAKE_TOOL_CMAKE=!MAKE_TOOL:\=/!"
        set "GENERATOR=Ninja"
        goto :found_make
    )
)

:found_make
if not defined MAKE_TOOL (
    echo [ERROR] Neither mingw32-make nor ninja found in PATH.
    echo Please install one of them:
    echo   - MinGW Make: comes with MinGW-w64
    echo   - Ninja: https://ninja-build.org/
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

if not exist "%GRAPHVIZ_DIR%" (
    echo [INFO] Creating directory for graphviz files: "%GRAPHVIZ_DIR%"
    mkdir "%GRAPHVIZ_DIR%" >nul 2>&1
)

rem =========================================
rem 🔍 Detect Qt installation (optional, for info only)
rem =========================================
set "QT_INFO="
set "QT_DIR_CMAKE="
set "CMAKE_PREFIX_PATH_CMAKE="

if defined Qt6_DIR (
    set "QT_INFO=Qt6_DIR: %Qt6_DIR%"
    rem Convert backslashes to forward slashes for CMake
    set "QT_DIR_CMAKE=!Qt6_DIR:\=/!"
) else if defined CMAKE_PREFIX_PATH (
    set "QT_INFO=CMAKE_PREFIX_PATH: %CMAKE_PREFIX_PATH%"
    rem Convert backslashes to forward slashes for CMake
    set "CMAKE_PREFIX_PATH_CMAKE=!CMAKE_PREFIX_PATH:\=/!"
) else (
    set "QT_INFO=Not set (will use system default or CMake will search)"
)

rem =========================================
rem 🔧 Convert other paths for CMake
rem =========================================
set "SOURCE_DIR_CMAKE=!SOURCE_DIR:\=/!"
set "BUILD_DIR_CMAKE=!BUILD_DIR:\=/!"
set "INSTALL_DIR_CMAKE=!INSTALL_DIR:\=/!"
set "GRAPHVIZ_FILE_CMAKE=!GRAPHVIZ_FILE:\=/!"

rem =========================================
rem 📋 Display configuration summary
rem =========================================
echo ****************************************************
echo Start generating project on Windows with MinGW
echo ROOT_DIR   : "%ROOT_DIR%"
echo SOURCE_DIR : "%SOURCE_DIR%"
echo BUILD_DIR  : "%BUILD_DIR%"
echo INSTALL_DIR: "%INSTALL_DIR%"
echo CMAKE      : "%CMAKE%"
echo GCC        : "%MINGW_GCC%"
echo G++        : "%MINGW_GXX%"
echo Generator  : %GENERATOR%
echo Make Tool  : "%MAKE_TOOL%"
echo Qt Info    : %QT_INFO%
echo ****************************************************

rem =========================================
rem 🚀 Run CMake configuration with MinGW
rem =========================================
echo [INFO] Running CMake configuration...

"%CMAKE%" -S "%SOURCE_DIR_CMAKE%" -B "%BUILD_DIR_CMAKE%" ^
    -G "%GENERATOR%" ^
    -DCMAKE_C_COMPILER="%MINGW_GCC_CMAKE%" ^
    -DCMAKE_CXX_COMPILER="%MINGW_GXX_CMAKE%" ^
    --graphviz="%GRAPHVIZ_FILE_CMAKE%" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="%BUILD_DIR_CMAKE%\bin" ^
    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="%BUILD_DIR_CMAKE%\bin" ^
    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="%BUILD_DIR_CMAKE%\bin" ^
    -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR_CMAKE%" ^
    -DCMAKE_VERBOSE_MAKEFILE=ON ^
    -DI18N_UPDATE_TS=ON

if defined MAKE_TOOL_CMAKE (
    "%CMAKE%" -S "%SOURCE_DIR_CMAKE%" -B "%BUILD_DIR_CMAKE%" ^
        -DCMAKE_MAKE_PROGRAM="%MAKE_TOOL_CMAKE%"
)

if defined QT_DIR_CMAKE (
    "%CMAKE%" -S "%SOURCE_DIR_CMAKE%" -B "%BUILD_DIR_CMAKE%" ^
        -DQt6_DIR="%QT_DIR_CMAKE%"
) else if defined CMAKE_PREFIX_PATH_CMAKE (
    "%CMAKE%" -S "%SOURCE_DIR_CMAKE%" -B "%BUILD_DIR_CMAKE%" ^
        -DCMAKE_PREFIX_PATH="%CMAKE_PREFIX_PATH_CMAKE%"
)

set "EXIT_CODE=%errorlevel%"

rem =========================================
rem 📦 Result summary
rem =========================================
if not "%EXIT_CODE%"=="0" (
    echo ****************************************************
    echo [ERROR] CMake configuration failed with code %EXIT_CODE%.
    echo ****************************************************
    echo.
    echo 💡 Common issues:
    echo   - Qt not found: Set Qt6_DIR or CMAKE_PREFIX_PATH
    echo     Example: set CMAKE_PREFIX_PATH=C:\Qt\6.5.3\mingw_64
    echo   - Wrong compiler: Ensure MinGW-w64 is in PATH
    echo   - Missing dependencies: Check CMake output above
    echo ****************************************************
    exit /b %EXIT_CODE%
) else (
    echo ****************************************************
    echo ✅ CMake project generated successfully.
    echo Build files are located in: "%BUILD_DIR%"
    echo.
    echo 📝 Next steps:
    echo   1. Build: win_x64_build_cmake_qt_mingw.bat
    echo   2. Or manually: cd "%BUILD_DIR%" ^&^& %MAKE_TOOL%
    echo ****************************************************
)

endlocal
exit /b 0