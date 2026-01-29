# =============================================================================
# Extract Large Files Script
# =============================================================================
# Extracts large binary files from .zip archives.
# These files are stored compressed to fit GitHub's 100MB file size limit.
#
# Usage:
#   .\extract_large_files.ps1                  # Extract all platforms
#   .\extract_large_files.ps1 -Platform android
#   .\extract_large_files.ps1 -Platform windows
# =============================================================================

param(
    [ValidateSet("all", "android", "windows")]
    [string]$Platform = "all"
)

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$baseDir = Split-Path -Parent $scriptDir
$configPath = Join-Path $baseDir "large_files.json"

if (!(Test-Path $configPath)) {
    Write-Error "Config file not found: $configPath"
    exit 1
}

$config = Get-Content $configPath -Raw | ConvertFrom-Json
$extractedCount = 0
$skippedCount = 0

Write-Host "=== Extracting Large Files ===" -ForegroundColor Cyan
Write-Host "Platform: $Platform"
Write-Host ""

foreach ($file in $config.files) {
    if ($Platform -ne "all" -and $file.platform -ne $Platform) {
        continue
    }
    
    $originalPath = Join-Path $baseDir $file.path
    $zipPath = "$originalPath.zip"
    
    if (!(Test-Path $zipPath)) {
        Write-Host "[SKIP] Zip not found: $($file.path).zip" -ForegroundColor Yellow
        $skippedCount++
        continue
    }
    
    if (Test-Path $originalPath) {
        Write-Host "[SKIP] Already exists: $($file.path)" -ForegroundColor Gray
        $skippedCount++
        continue
    }
    
    Write-Host "[EXTRACT] $($file.path)" -ForegroundColor Green
    $destDir = Split-Path -Parent $originalPath
    
    try {
        Expand-Archive -Path $zipPath -DestinationPath $destDir -Force
        $extractedCount++
    } catch {
        Write-Error "Failed to extract: $zipPath"
        Write-Error $_.Exception.Message
    }
}

Write-Host ""
Write-Host "=== Summary ===" -ForegroundColor Cyan
Write-Host "Extracted: $extractedCount"
Write-Host "Skipped: $skippedCount"
Write-Host "Done."
