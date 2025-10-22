@echo off
setlocal enabledelayedexpansion
set ROOT_DIR=%~dp0..\
set BUILD_DIR=%ROOT_DIR%build
set GRAPHVIZ_FILE=%BUILD_DIR%\cmake_graph\cmake_graph.dot

@REM set CMAKE=%ROOT_DIR%tools\platforms\windows\x64\cmake\bin\cmake.exe
set SOURCE_DIR=%ROOT_DIR%.

rem ==============================
rem Try find system cmake first
rem ==============================
where cmake >nul 2>nul
if %errorlevel%==0 (
    for /f "delims=" %%i in ('where cmake') do set "CMAKE=%%i"
) else (
    echo [WARN] System cmake not found, fallback to bundled version.
    set CMAKE=%ROOT_DIR%\tools\platforms\windows\x64\cmake\bin\cmake.exe
)

echo ****************************************************
echo Start generate project on Windows
echo ROOT_DIR is %ROOT_DIR%
echo ****************************************************

echo =========================================================
echo CMAKE: %CMAKE%
echo SOURCE_DIR: "%SOURCE_DIR%"
echo BUILD_DIR : "%BUILD_DIR%"
echo =========================================================

if NOT exist %BUILD_DIR% (
    mkdir %BUILD_DIR%
)

"%CMAKE%" -S "%SOURCE_DIR%" -B "%BUILD_DIR%" ^
          -G"Visual Studio 17 2022" -A "x64" ^
          --graphviz="%GRAPHVIZ_FILE%" ^
          -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="%BUILD_DIR%\bin" ^
          -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="%BUILD_DIR%\bin" ^
          -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="%BUILD_DIR%\bin" ^
          -DCMAKE_INSTALL_PREFIX="%ROOT_DIR%install" ^
          -DCMAKE_VERBOSE_MAKEFILE=ON

set EXIT_CODE=%errorlevel%

echo ****************************************************
echo Finish generate project on Windows
echo ROOT_DIR is %ROOT_DIR%
echo BUILD_DIR is %BUILD_DIR%
echo ****************************************************

rem ==============================
rem Post summary
rem ==============================
if %EXIT_CODE% NEQ 0 (
    echo [ERROR] CMake configuration failed with code %EXIT_CODE%.
    exit /b %EXIT_CODE%
) else (
    echo ****************************************************
    echo CMake project generated successfully.
    echo Build files at: %BUILD_DIR%
    echo ****************************************************
)

endlocal
exit /B %EXIT_CODE%
