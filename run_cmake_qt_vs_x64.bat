@echo off

echo =========================================================

set ROOT_DIR=%~dp0
set PYTHONPATH=%ROOT_DIR%

set BUILD_DIR=%ROOT_DIR%build_x64
set GRAPHVIZ_FILE=%BUILD_DIR%\cmake_graph\cmake_graph.dot
if NOT exist %BUILD_DIR% (
    mkdir %BUILD_DIR%
)

set CMAKE=%ROOT_DIR%tools\platforms\windows\x64\cmake\bin\cmake.exe
set SOURCE_DIR=%ROOT_DIR%.

echo CMAKE: %CMAKE%
echo SOURCE_DIR: "%SOURCE_DIR%"
echo BUILD_DIR : "%BUILD_DIR%"
echo =========================================================

"%CMAKE%" -S "%SOURCE_DIR%" -B "%BUILD_DIR%" ^
          -G"Visual Studio 17 2022" -A "x64" ^
          --graphviz="%GRAPHVIZ_FILE%" ^
          -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="%BUILD_DIR%\bin" ^
          -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="%BUILD_DIR%\bin" ^
          -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="%BUILD_DIR%\lib" ^
          -DCMAKE_INSTALL_PREFIX="%ROOT_DIR%install"^
          -DCTEST_CATCH2_ENABLE=True ^
          -DENABLE_FAST_RELEASE_BUILD=YES 

set EXIT_CODE=%errorlevel%


exit /B %EXIT_CODE%
