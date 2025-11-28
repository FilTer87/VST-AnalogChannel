# ============================================================================
# AnalogChannel Windows Build Script
# ============================================================================
# This script:
#   1. Updates version in all files (from .jucer)
#   2. Builds the plugin for Windows using MSBuild
#
# Usage:
#   .\build-windows.ps1 [Debug|Release]
#
# Default: Release
#
# Requirements:
#   - Visual Studio 2022 (or MSBuild tools)
#   - Projucer must have generated the VS project
# ============================================================================

param(
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"

# Colors
function Write-Color {
    param([string]$Text, [string]$Color = "White")
    Write-Host $Text -ForegroundColor $Color
}

Write-Color "============================================" "Cyan"
Write-Color "AnalogChannel Windows Build" "Cyan"
Write-Color "============================================`n" "Cyan"

# ============================================================================
# Validate configuration
# ============================================================================

if ($Configuration -ne "Debug" -and $Configuration -ne "Release") {
    Write-Color "ERROR: Invalid configuration '$Configuration'" "Red"
    Write-Color "Usage: .\build-windows.ps1 [Debug|Release]" "Yellow"
    exit 1
}

Write-Color "Configuration: $Configuration`n" "Cyan"

# ============================================================================
# Step 1: Update version in all files
# ============================================================================

Write-Color "Step 1: Updating version in all files...`n" "Cyan"

if (Test-Path "update-version.ps1") {
    & .\update-version.ps1
    Write-Host ""
} else {
    Write-Color "Warning: update-version.ps1 not found, skipping version update`n" "Yellow"
}

# ============================================================================
# Step 2: Check if Visual Studio project exists
# ============================================================================

$VSProject = "Builds\VisualStudio2022\AnalogChannel.sln"

if (-not (Test-Path $VSProject)) {
    Write-Color "ERROR: Visual Studio solution not found at $VSProject" "Red"
    Write-Color "Please run Projucer first to generate the Visual Studio project." "Yellow"
    exit 1
}

# ============================================================================
# Step 3: Locate MSBuild
# ============================================================================

Write-Color "Step 2: Locating MSBuild..." "Cyan"

# Try to find MSBuild via vswhere (Visual Studio 2017+)
$vswherePath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

if (Test-Path $vswherePath) {
    $vsPath = & $vswherePath -latest -products * -requires Microsoft.Component.MSBuild -property installationPath

    if ($vsPath) {
        $msbuildPath = Join-Path $vsPath "MSBuild\Current\Bin\MSBuild.exe"

        if (-not (Test-Path $msbuildPath)) {
            # Try older VS versions
            $msbuildPath = Join-Path $vsPath "MSBuild\15.0\Bin\MSBuild.exe"
        }
    }
}

# Fallback: try common paths
if (-not $msbuildPath -or -not (Test-Path $msbuildPath)) {
    $commonPaths = @(
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe",
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe",
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
    )

    foreach ($path in $commonPaths) {
        if (Test-Path $path) {
            $msbuildPath = $path
            break
        }
    }
}

if (-not $msbuildPath -or -not (Test-Path $msbuildPath)) {
    Write-Color "ERROR: MSBuild not found!" "Red"
    Write-Color "Please install Visual Studio 2022 or Visual Studio Build Tools" "Yellow"
    Write-Color "Download from: https://visualstudio.microsoft.com/downloads/" "Yellow"
    exit 1
}

Write-Color "  Found MSBuild: $msbuildPath" "Green"
Write-Host ""

# ============================================================================
# Step 4: Build
# ============================================================================

Write-Color "Step 3: Building AnalogChannel..." "Cyan"
Write-Host "  Solution: $VSProject"
Write-Host "  Configuration: $Configuration"
Write-Host "  Platform: x64"
Write-Host ""

# Build VST3
& $msbuildPath $VSProject `
    /p:Configuration=$Configuration `
    /p:Platform=x64 `
    /t:Build `
    /m `
    /v:minimal

if ($LASTEXITCODE -ne 0) {
    Write-Color "`nBuild failed!" "Red"
    exit 1
}

Write-Color "`nBuild completed successfully!`n" "Green"

# ============================================================================
# Step 5: Locate built plugins
# ============================================================================

$BuildDir = "Builds\VisualStudio2022\x64\$Configuration\VST3"
$VST3Path = "$BuildDir\AnalogChannel.vst3\Contents\x86_64-win\AnalogChannel.vst3"

Write-Color "Step 4: Locating built plugins..." "Cyan"

if (Test-Path $VST3Path) {
    Write-Color "  ✓ Found VST3: $VST3Path" "Green"
} else {
    Write-Color "  ✗ VST3 not found at expected location" "Red"
    Write-Color "    Expected: $VST3Path" "Yellow"
    exit 1
}

Write-Host ""

# ============================================================================
# Step 6: Copy to release folder
# ============================================================================

Write-Color "Step 5: Copying plugins to release folder..." "Cyan"

# Read version from .jucer file
$jucerContent = Get-Content "AnalogChannel.jucer" -Raw

if ($jucerContent -match '<JUCERPROJECT[^>]+version="([^"]+)"') {
    $Version = $Matches[1]
} else {
    Write-Color "ERROR: Could not read version from .jucer file" "Red"
    exit 1
}

$ReleaseDir = "Builds\AnalogChannel_Release_v$Version"

# Create release directory if it doesn't exist
if (-not (Test-Path $ReleaseDir)) {
    New-Item -ItemType Directory -Path $ReleaseDir | Out-Null
    Write-Color "  Created release directory: $ReleaseDir" "Green"
} else {
    Write-Color "  Release directory exists: $ReleaseDir" "Yellow"
}

# Create Windows subdirectory
$WindowsDir = "$ReleaseDir\x86_64-win"
if (-not (Test-Path $WindowsDir)) {
    New-Item -ItemType Directory -Path $WindowsDir | Out-Null
}

# Copy VST3 (Windows) - copy the entire bundle
$VST3BundlePath = "$BuildDir\AnalogChannel.vst3"
$DestVST3Path = "$WindowsDir\AnalogChannel.vst3"

if (Test-Path $VST3BundlePath) {
    Write-Host "  Copying: AnalogChannel.vst3 → x86_64-win\"

    # Remove existing if present
    if (Test-Path $DestVST3Path) {
        Remove-Item -Recurse -Force $DestVST3Path
    }

    Copy-Item -Recurse -Force $VST3BundlePath $DestVST3Path
    Write-Color "  [OK] VST3 copied to x86_64-win\" "Green"
}

# Copy README to release root
$ReadmeFile = "README.txt"
if (Test-Path $ReadmeFile) {
    Write-Host "  Copying: README.txt → release root"
    Copy-Item -Force $ReadmeFile "$ReleaseDir\"
    Write-Color "  [OK] README copied to release root" "Green"
} else {
    Write-Color "  [WARNING] README.txt not found in project root" "Yellow"
}

# Copy LICENSE to release root
$LicenseFile = "LICENSE.md"
if (Test-Path $LicenseFile) {
    Write-Host "  Copying: LICENSE.md → release root"
    Copy-Item -Force $LicenseFile "$ReleaseDir\"
    Write-Color "  [OK] LICENSE copied to release root" "Green"
} else {
    Write-Color "  [WARNING] LICENSE.md not found in project root" "Yellow"
}

Write-Host ""

# ============================================================================
# Step 7: Generate PDF Manual
# ============================================================================

Write-Color "Step 6: Generating PDF manual..." "Cyan"

# Check if pandoc is installed
$pandocInstalled = $null
try {
    $pandocInstalled = Get-Command pandoc -ErrorAction Stop
} catch {
    $pandocInstalled = $null
}

if ($pandocInstalled) {
    Write-Host "  Pandoc found, generating PDF..."

    # Create temp directory for PDF generation
    $TempDir = "$ReleaseDir\.temp_manual"
    if (-not (Test-Path $TempDir)) {
        New-Item -ItemType Directory -Path $TempDir | Out-Null
    }

    # Copy UserManual.md and images
    if (Test-Path "Docs\UserManual.md") {
        Copy-Item -Force "Docs\UserManual.md" "$TempDir\"

        if (Test-Path "Docs\images") {
            Copy-Item -Recurse -Force "Docs\images" "$TempDir\"
        }

        # Remove TOC (lines 10-47) from copied markdown
        $lines = Get-Content "$TempDir\UserManual.md"
        $before = $lines[0..8]  # Lines 1-9
        $after = $lines[47..($lines.Length-1)]  # Line 48 onwards
        $before + $after | Set-Content "$TempDir\UserManual.md"

        # Generate PDF with platform-specific fonts
        # Convert to absolute path BEFORE changing directory
        $pdfOutput = Join-Path (Get-Location) "$ReleaseDir\AnalogChannel_UserManual.pdf"
        $pdfOutput = [System.IO.Path]::GetFullPath($pdfOutput)

        Push-Location $TempDir

        # Redirect pandoc output to suppress warnings (they're informational only)
        $pandocOutput = & pandoc .\UserManual.md -o $pdfOutput `
            --pdf-engine=xelatex `
            --toc `
            --toc-depth=3 `
            --number-sections `
            -V geometry:margin=2cm `
            -V mainfont="Segoe UI" `
            -V monofont="Consolas" `
            -V fontsize=11pt `
            -V linkcolor=blue `
            --syntax-highlighting=tango 2>&1

        Pop-Location

        # Small delay to ensure file is written to disk
        Start-Sleep -Milliseconds 100

        # Check if PDF was actually created (ignore exit code, pandoc returns non-zero on warnings)
        if (Test-Path $pdfOutput) {
            $pdfSize = "{0:N2} KB" -f ((Get-Item $pdfOutput).Length / 1KB)
            Write-Color "  [OK] PDF manual generated: AnalogChannel_UserManual.pdf ($pdfSize)" "Green"

            # Show warnings if there were any, but don't fail
            if ($LASTEXITCODE -ne 0) {
                Write-Color "  [INFO] Pandoc reported warnings (PDF created successfully anyway)" "Yellow"
            }
        } else {
            Write-Color "  [ERROR] PDF generation failed! Check pandoc output:" "Red"
            Write-Host $pandocOutput
            Write-Color "  Manual must be generated manually." "Yellow"
            Write-Color "  See: InternalDocs\Generate_PDF_manual.md" "Yellow"
        }

        # Clean up temp directory
        Remove-Item -Recurse -Force $TempDir
    } else {
        Write-Color "  [ERROR] Docs\UserManual.md not found!" "Red"
    }
} else {
    Write-Color "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" "Red"
    Write-Color "⚠️  WARNING: Pandoc not installed!" "Yellow"
    Write-Color "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" "Red"
    Write-Host ""
    Write-Color "PDF manual will NOT be generated automatically." "Yellow"
    Write-Host ""
    Write-Color "To install pandoc:" "Cyan"
    Write-Host "  Windows: https://pandoc.org/installing.html"
    Write-Host ""
    Write-Color "You can generate the PDF manually later:" "Cyan"
    Write-Host "  See: InternalDocs\Generate_PDF_manual.md"
    Write-Host ""
    Write-Color "Build will continue without PDF..." "Yellow"
    Write-Color "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" "Red"
}

Write-Host ""

# ============================================================================
# Summary
# ============================================================================

Write-Color "============================================" "Cyan"
Write-Color "Build completed successfully!" "Green"
Write-Host ""
Write-Color "Release folder:" "Cyan"
Write-Host "  $ReleaseDir"
Write-Host ""
Write-Color "Files copied:" "Cyan"
Write-Host "  ✓ x86_64-win\AnalogChannel.vst3 (Windows x64)"
if (Test-Path "$ReleaseDir\README.txt") {
    Write-Host "  ✓ README.txt (Documentation)"
}
if (Test-Path "$ReleaseDir\LICENSE.md") {
    Write-Host "  ✓ LICENSE.md (License)"
}
if (Test-Path "$ReleaseDir\AnalogChannel_UserManual.pdf") {
    Write-Host "  ✓ AnalogChannel_UserManual.pdf (User Manual)"
}
Write-Host ""
Write-Color "Next steps:" "Cyan"
Write-Host "  1. Build macOS version (run build-and-sign-macos.sh on macOS)"
Write-Host "  2. Add documentation to $ReleaseDir"
Write-Host "  3. Create ZIP from $ReleaseDir"
