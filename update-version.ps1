# ============================================================================
# AnalogChannel Version Update Script
# ============================================================================
# This script reads the version from AnalogChannel.jucer and updates it
# across all documentation and source files.
#
# Usage:
#   .\update-version.ps1
#
# Or manually specify a version:
#   .\update-version.ps1 -Version "0.5.0"
# ============================================================================

param(
    [string]$Version = $null
)

$ErrorActionPreference = "Stop"

# ============================================================================
# 1. Read version from .jucer if not provided
# ============================================================================

if (-not $Version) {
    Write-Host "Reading version from AnalogChannel.jucer..." -ForegroundColor Cyan

    $jucerPath = "AnalogChannel.jucer"
    if (-not (Test-Path $jucerPath)) {
        Write-Host "ERROR: AnalogChannel.jucer not found!" -ForegroundColor Red
        exit 1
    }

    $jucerContent = Get-Content $jucerPath -Raw

    # Match version in JUCERPROJECT tag (not XML declaration)
    if ($jucerContent -match '<JUCERPROJECT[^>]+version="([^"]+)"') {
        $Version = $Matches[1]
        Write-Host "Found version: $Version" -ForegroundColor Green
    } else {
        Write-Host "ERROR: Could not find version in JUCERPROJECT tag!" -ForegroundColor Red
        exit 1
    }
}

Write-Host "`nUpdating all files to version: $Version" -ForegroundColor Yellow
Write-Host "============================================`n" -ForegroundColor Yellow

# ============================================================================
# 2. Define files to update with their search patterns
# ============================================================================

$filesToUpdate = @(
    # Docs/UserManual.md - Line 4 (header)
    @{
        Path = "Docs\UserManual.md"
        Pattern = '(?<=\*\*Version )[0-9]+\.[0-9]+(\.[0-9]+)?(?=\*\*)'
        Replacement = $Version
        Description = "User Manual - Header (line 4)"
    },
    # Docs/UserManual.md - Line 1096 (Plugin Information section)
    @{
        Path = "Docs\UserManual.md"
        Pattern = '(?<=- \*\*Version\*\*: )[0-9]+\.[0-9]+(\.[0-9]+)?'
        Replacement = $Version
        Description = "User Manual - Plugin Information (line 1096)"
    },
    # Docs/UserManual.md - Line 1258 (footer)
    @{
        Path = "Docs\UserManual.md"
        Pattern = '(?<=\*AnalogChannel User Manual v)[0-9]+\.[0-9]+(\.[0-9]+)?(?= \|)'
        Replacement = $Version
        Description = "User Manual - Footer (line 1258)"
    },
    # InternalDocs/README_Release.txt - Line 4 (header)
    @{
        Path = "InternalDocs\README_Release.txt"
        Pattern = '(?<=Version )[0-9]+\.[0-9]+(\.[0-9]+)?'
        Replacement = $Version
        Description = "README_Release.txt - Header (line 4)"
    },
    # Source/PluginEditor.cpp - Line 293 (infoLabel.setText)
    @{
        Path = "Source\PluginEditor.cpp"
        Pattern = '(?<=AnalogChannel v)[0-9]+\.[0-9]+(\.[0-9]+)?(?=\\n)'
        Replacement = $Version
        Description = "PluginEditor.cpp - About dialog (line 293)"
    }
)

# ============================================================================
# 3. Update each file
# ============================================================================

$updatedCount = 0
$skippedCount = 0
$errorCount = 0

foreach ($file in $filesToUpdate) {
    $filePath = $file.Path
    $pattern = $file.Pattern
    $replacement = $file.Replacement
    $description = $file.Description

    Write-Host "Processing: $description" -ForegroundColor Cyan
    Write-Host "  File: $filePath"

    if (-not (Test-Path $filePath)) {
        Write-Host "  [ERROR] File not found!" -ForegroundColor Red
        $errorCount++
        continue
    }

    $content = Get-Content $filePath -Raw -Encoding UTF8

    if ($content -match $pattern) {
        $oldMatch = $Matches[0]
        $newContent = $content -replace $pattern, $replacement

        # Preserve original encoding and line endings
        Set-Content $filePath -Value $newContent -NoNewline -Encoding UTF8

        Write-Host "  [OK] Updated: '$oldMatch' -> '$replacement'" -ForegroundColor Green
        $updatedCount++
    } else {
        Write-Host "  [WARNING] Pattern not found in file!" -ForegroundColor Yellow
        Write-Host "  Pattern: $pattern" -ForegroundColor Yellow
        $skippedCount++
    }

    Write-Host ""
}

# ============================================================================
# 4. Summary
# ============================================================================

Write-Host "============================================" -ForegroundColor Yellow
Write-Host "Summary:" -ForegroundColor Yellow
Write-Host "  Version: $Version" -ForegroundColor Cyan
Write-Host "  Updated: $updatedCount files" -ForegroundColor Green

if ($skippedCount -gt 0) {
    Write-Host "  Skipped: $skippedCount files" -ForegroundColor Yellow
}

if ($errorCount -gt 0) {
    Write-Host "  Errors:  $errorCount files" -ForegroundColor Red
    Write-Host "`nVersion update completed with errors!" -ForegroundColor Red
    exit 1
} else {
    Write-Host "`nVersion update completed successfully!" -ForegroundColor Green
    Write-Host "`nNext steps:" -ForegroundColor Cyan
    Write-Host "  1. Review changes: git diff"
    Write-Host "  2. Open Projucer and save (to regenerate build files)"
    Write-Host "  3. Rebuild in Visual Studio"
    Write-Host "  4. Commit: git add . && git commit -m 'Bump version to $Version'"
    Write-Host "  5. Tag: git tag v$Version"
    Write-Host "  6. Push: git push && git push --tags"
}
