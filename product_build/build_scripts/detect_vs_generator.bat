@echo off
setlocal enabledelayedexpansion

rem =========================================
rem Purpose: Detect the latest installed Visual Studio
rem          using vswhere, and output the corresponding
rem          CMake generator string (e.g. "Visual Studio 17 2022 -A x64").
rem
rem If no valid Visual Studio is detected, print an error
rem and exit with code 1.
rem =========================================

rem --------------------------
rem Define the path to vswhere.exe.
rem This tool is installed with Visual Studio, typically here:
rem C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe
rem --------------------------
set "VS_WHERE_PATH=C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"

rem --------------------------
rem Check if vswhere.exe exists.
rem If not, print an error and exit.
rem --------------------------
if not exist "%VS_WHERE_PATH%" (
    echo [ERROR] vswhere.exe not found. Please install Visual Studio.
    exit /b 1
)

rem --------------------------
rem Use vswhere to get the latest Visual Studio installation version.
rem Property: installationVersion (e.g. "17.0.0")
rem --------------------------
set "VS_INSTALL_VERSION="
for /f "usebackq delims=" %%i in (`"%VS_WHERE_PATH%" -latest -products * -requires Microsoft.Component.MSBuild -property installationVersion 2^>nul`) do (
    set "VS_INSTALL_VERSION=%%i"
)

rem --------------------------
rem Use vswhere to get the installation path of Visual Studio.
rem Property: installationPath
rem --------------------------
set "VS_INSTALL_PATH="
for /f "usebackq delims=" %%j in (`"%VS_WHERE_PATH%" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath 2^>nul`) do (
    set "VS_INSTALL_PATH=%%j"
)

rem --------------------------
rem Extract the major version number from the installationVersion.
rem For example: "17.0.0" --> "17"
rem --------------------------
set "VS_MAJOR="
if defined VS_INSTALL_VERSION (
    for /f "tokens=1 delims=." %%a in ("%VS_INSTALL_VERSION%") do (
        set "VS_MAJOR=%%a"
    )
)

rem --------------------------
rem Based on the major version, map to the corresponding
rem CMake generator string.
rem Supported: VS 2017 (15), VS 2019 (16), VS 2022 (17)
rem --------------------------
set "VS_GENERATOR="
if "!VS_MAJOR!"=="17" (
    set "VS_GENERATOR=Visual Studio 17 2022 -A x64"
) else if "!VS_MAJOR!"=="16" (
    set "VS_GENERATOR=Visual Studio 16 2019 -A x64"
) else if "!VS_MAJOR!"=="15" (
    set "VS_GENERATOR=Visual Studio 15 2017 -A x64"
)

rem --------------------------
rem If a valid generator is detected, output it and exit successfully.
rem This is the only output when detection succeeds.
rem --------------------------
if defined VS_GENERATOR (
    echo %VS_GENERATOR%
    exit /b 0
) else (
    rem --------------------------
    rem If no valid generator could be determined, output a simple error
    rem and exit with failure code.
    rem --------------------------
    echo [ERROR] Unable to detect a valid Visual Studio generator.
    exit /b 1
)