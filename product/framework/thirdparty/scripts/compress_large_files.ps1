# =============================================================================
# Compress Large Files Script
# =============================================================================
# Compresses large binary files to .zip archives for Git storage.
# Use this script when updating the original binary files.
#
# Usage:
#   .\compress_large_files.ps1                  # Compress all
#   .\compress_large_files.ps1 -Platform android
#   .\compress_large_files.ps1 -Platform windows
#   .\compress_large_files.ps1 -Force           # Overwrite existing zips
# =============================================================================

param(
    [ValidateSet("all", "android", "windows")]
    [string]$Platform = "all",
    [switch]$Force
)

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$baseDir = Split-Path -Parent $scriptDir
$configPath = Join-Path $baseDir "large_files.json"

if (!(Test-Path $configPath)) {
    Write-Error "Config file not found: $configPath"
    exit 1
}

$config = Get-Content $configPath -Raw | ConvertFrom-Json
$compressedCount = 0
$skippedCount = 0
$totalOriginal = 0
$totalCompressed = 0

Write-Host "=== Compressing Large Files ===" -ForegroundColor Cyan
Write-Host "Platform: $Platform"
Write-Host "Force: $Force"
Write-Host ""

foreach ($file in $config.files) {
    if ($Platform -ne "all" -and $file.platform -ne $Platform) {
        continue
    }
    
    $originalPath = Join-Path $baseDir $file.path
    $zipPath = "$originalPath.zip"
    
    if (!(Test-Path $originalPath)) {
        Write-Host "[SKIP] Original not found: $($file.path)" -ForegroundColor Yellow
        $skippedCount++
        continue
    }
    
    if ((Test-Path $zipPath) -and !$Force) {
        Write-Host "[SKIP] Zip exists (use -Force to overwrite): $($file.path).zip" -ForegroundColor Gray
        $skippedCount++
        continue
    }
    
    Write-Host "[COMPRESS] $($file.path)" -ForegroundColor Green
    
    try {
        Compress-Archive -Path $originalPath -DestinationPath $zipPath -Force
        
        $originalSize = (Get-Item $originalPath).Length
        $compressedSize = (Get-Item $zipPath).Length
        $ratio = [math]::Round((1 - $compressedSize / $originalSize) * 100, 1)
        
        $originalMB = [math]::Round($originalSize / 1MB, 2)
        $compressedMB = [math]::Round($compressedSize / 1MB, 2)
        
        Write-Host "  $originalMB MB -> $compressedMB MB (saved $ratio%)" -ForegroundColor DarkGray
        
        $totalOriginal += $originalSize
        $totalCompressed += $compressedSize
        $compressedCount++
    } catch {
        Write-Error "Failed to compress: $originalPath"
        Write-Error $_.Exception.Message
    }
}

Write-Host ""
Write-Host "=== Summary ===" -ForegroundColor Cyan
Write-Host "Compressed: $compressedCount files"
Write-Host "Skipped: $skippedCount files"

if ($totalOriginal -gt 0) {
    $totalOriginalMB = [math]::Round($totalOriginal / 1MB, 2)
    $totalCompressedMB = [math]::Round($totalCompressed / 1MB, 2)
    $totalRatio = [math]::Round((1 - $totalCompressed / $totalOriginal) * 100, 1)
    Write-Host "Total: $totalOriginalMB MB -> $totalCompressedMB MB (saved $totalRatio%)"
}

Write-Host "Done."
