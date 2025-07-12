# OpenAuto Modern Logger Build Verification Script (PowerShell)
#
# This script helps identify and fix common build issues when migrating
# to the modern logger system.
#

Write-Host "=== OpenAuto Modern Logger Build Verification ===" -ForegroundColor Cyan
Write-Host

$issues = 0

# Check for legacy EventBus files
Write-Host "Checking for legacy EventBus files..." -ForegroundColor Yellow
$legacyEventBus = Get-ChildItem -Recurse -Filter "*EventBus*" -ErrorAction SilentlyContinue | Where-Object { $_.FullName -notmatch "\\build\\" }
if ($legacyEventBus) {
    Write-Host "WARNING: Legacy EventBus files found:" -ForegroundColor Red
    $legacyEventBus | ForEach-Object { Write-Host "  - $($_.FullName)" }
    $issues++
} else {
    Write-Host "OK: No legacy EventBus files found" -ForegroundColor Green
}
Write-Host

# Check for legacy includes in source files
Write-Host "Checking for legacy include statements..." -ForegroundColor Yellow
$legacyIncludes = Get-ChildItem -Recurse -Include "*.cpp", "*.hpp", "*.h" -ErrorAction SilentlyContinue | 
    Where-Object { $_.FullName -notmatch "\\build\\" } |
    Select-String -Pattern "f1x.*EventBus" -ErrorAction SilentlyContinue

if ($legacyIncludes) {
    Write-Host "WARNING: Legacy include statements found:" -ForegroundColor Red
    $legacyIncludes | Select-Object -First 5 | ForEach-Object { 
        Write-Host "  - $($_.Filename):$($_.LineNumber): $($_.Line.Trim())" 
    }
    if ($legacyIncludes.Count -gt 5) {
        Write-Host "  ... and $($legacyIncludes.Count - 5) more"
    }
    $issues++
} else {
    Write-Host "OK: No legacy include statements found" -ForegroundColor Green
}
Write-Host

# Check for legacy Log.hpp includes
Write-Host "Checking for legacy logging includes..." -ForegroundColor Yellow
$legacyLogIncludes = Get-ChildItem -Recurse -Include "*.cpp", "*.hpp", "*.h" -ErrorAction SilentlyContinue | 
    Where-Object { $_.FullName -notmatch "\\build\\" } |
    Select-String -Pattern "f1x/openauto/Common/Log\.hpp" -ErrorAction SilentlyContinue

if ($legacyLogIncludes) {
    Write-Host "INFO: Legacy logging includes found (migration in progress):" -ForegroundColor Yellow
    Write-Host "  Found $($legacyLogIncludes.Count) files with legacy logging includes"
    Write-Host "  These should be migrated to: #include ""modern/Logger.hpp"""
} else {
    Write-Host "OK: No legacy logging includes found" -ForegroundColor Green
}
Write-Host

# Check for build directory
Write-Host "Checking build directory..." -ForegroundColor Yellow
if (Test-Path "build") {
    Write-Host "INFO: Build directory exists - recommend cleaning for migration" -ForegroundColor Yellow
    Write-Host "  Run: Remove-Item -Recurse -Force build"
} else {
    Write-Host "OK: No build directory (clean state)" -ForegroundColor Green
}
Write-Host

# Check for modern files
Write-Host "Checking for modern implementation files..." -ForegroundColor Yellow
$modernFiles = @(
    "include\modern\Logger.hpp",
    "include\modern\Event.hpp", 
    "include\modern\EventBus.hpp",
    "src\modern\Logger.cpp",
    "src\modern\Event.cpp",
    "src\modern\EventBus.cpp"
)

$missingFiles = @()
foreach ($file in $modernFiles) {
    if (-not (Test-Path $file)) {
        $missingFiles += $file
    }
}

if ($missingFiles.Count -eq 0) {
    Write-Host "OK: All modern implementation files found" -ForegroundColor Green
} else {
    Write-Host "WARNING: Missing modern implementation files:" -ForegroundColor Red
    $missingFiles | ForEach-Object { Write-Host "  - $_" }
    $issues++
}
Write-Host

# Summary and recommendations
if ($issues -eq 0) {
    Write-Host "SUCCESS: All critical checks passed!" -ForegroundColor Green
    Write-Host "Your project appears ready for modern logger build." -ForegroundColor Green
} else {
    Write-Host "WARNING: Found $issues potential issue(s)" -ForegroundColor Red
    Write-Host "Please review the warnings above before building." -ForegroundColor Red
}

Write-Host
Write-Host "Build Recommendations:" -ForegroundColor Cyan
Write-Host "1. Clean build: Remove-Item -Recurse -Force build; mkdir build; cd build"
Write-Host "2. Configure: cmake -DENABLE_MODERN_API=ON .."
Write-Host "3. Build: cmake --build . --verbose"
Write-Host "4. Test: .\logger_demo.exe (if built with ENABLE_LOGGER_DEMO=ON)"
