@echo off

set ROOT_DIR=%~dp0..\
set BUILD_DIR=%ROOT_DIR%build
set GRAPHVIZ_FILE=%BUILD_DIR%\cmake_graph\cmake_graph.dot

set CMAKE=%ROOT_DIR%tools\platforms\windows\x64\cmake\bin\cmake.exe
set SOURCE_DIR=%ROOT_DIR%.

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
exit /B %EXIT_CODE%
