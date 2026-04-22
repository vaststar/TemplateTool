@echo off
setlocal enabledelayedexpansion

set "INFO_JSON=%~dp0product\framework\version\product_info.json"

if not exist "%INFO_JSON%" (
    echo ERROR: %INFO_JSON% not found
    exit /b 1
)

:: Parse version fields from product_info.json via PowerShell
for /f "usebackq delims=" %%V in (`powershell -NoProfile -Command ^
    "$j = Get-Content '%INFO_JSON%' -Raw | ConvertFrom-Json; ^
     Write-Host \"$($j.version.major).$($j.version.minor).$($j.version.patch)\""`) do (
    set "VERSION=%%V"
)

set "TAG=v%VERSION%"
echo === Release Tag: %TAG% ===
echo Creating tag %TAG% on HEAD ...
git tag %TAG%
git push origin %TAG%

echo.
echo Done. Tag %TAG% pushed. GitHub Actions will create the release.
echo https://github.com/vaststar/TemplateTool/actions
