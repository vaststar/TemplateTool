@echo off
setlocal
call "%~dp0product_build\build.bat" %*
exit /b %errorlevel%
