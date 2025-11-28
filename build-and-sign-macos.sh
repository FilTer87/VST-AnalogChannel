#!/bin/bash
# ============================================================================
# AnalogChannel macOS Build & Self-Sign Script
# ============================================================================
# This script:
#   1. Updates version in all files (from .jucer)
#   2. Builds the plugin for macOS
#   3. Applies ad-hoc code signing
#
# Usage:
#   ./build-and-sign-macos.sh [Debug|Release]
#
# Default: Release
# ============================================================================

set -e

CONFIG="${1:-Release}"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${CYAN}============================================${NC}"
echo -e "${CYAN}AnalogChannel macOS Build & Sign${NC}"
echo -e "${CYAN}============================================${NC}\n"

# ============================================================================
# Validate configuration
# ============================================================================

if [ "$CONFIG" != "Debug" ] && [ "$CONFIG" != "Release" ]; then
    echo -e "${RED}ERROR: Invalid configuration '$CONFIG'${NC}"
    echo -e "${YELLOW}Usage: $0 [Debug|Release]${NC}"
    exit 1
fi

echo -e "${CYAN}Configuration: $CONFIG${NC}\n"

# ============================================================================
# Step 1: Update version in all files
# ============================================================================

echo -e "${CYAN}Step 1: Updating version in all files...${NC}\n"

if [ -f "update-version.sh" ]; then
    ./update-version.sh
    echo
else
    echo -e "${YELLOW}Warning: update-version.sh not found, skipping version update${NC}\n"
fi

# ============================================================================
# Step 2: Check if Xcode project exists
# ============================================================================

XCODE_PROJECT="Builds/MacOSX/AnalogChannel.xcodeproj"

if [ ! -d "$XCODE_PROJECT" ]; then
    echo -e "${RED}ERROR: Xcode project not found at $XCODE_PROJECT${NC}"
    echo -e "${YELLOW}Please run Projucer first to generate the Xcode project.${NC}"
    exit 1
fi

# ============================================================================
# Step 3: Build
# ============================================================================

echo -e "${CYAN}Step 2: Building AnalogChannel...${NC}"
echo "  Project: $XCODE_PROJECT"
echo "  Scheme: AnalogChannel - All"
echo "  Configuration: $CONFIG"
echo

cd Builds/MacOSX

xcodebuild -project AnalogChannel.xcodeproj \
  -scheme "AnalogChannel - All" \
  -configuration "$CONFIG" \
  clean build

if [ $? -ne 0 ]; then
    echo -e "\n${RED}Build failed!${NC}"
    exit 1
fi

echo -e "\n${GREEN}Build completed successfully!${NC}\n"

cd ../..

# ============================================================================
# Step 4: Locate built plugins
# ============================================================================

BUILD_DIR="Builds/MacOSX/build/$CONFIG"
VST3_PATH="$BUILD_DIR/AnalogChannel.vst3"
AU_PATH="$BUILD_DIR/AnalogChannel.component"

echo -e "${CYAN}Step 3: Locating built plugins...${NC}"

if [ -e "$VST3_PATH" ]; then
    echo -e "  ${GREEN}✓ Found VST3: $VST3_PATH${NC}"
else
    echo -e "  ${YELLOW}⚠ VST3 not found (may not be enabled)${NC}"
fi

if [ -e "$AU_PATH" ]; then
    echo -e "  ${GREEN}✓ Found AU: $AU_PATH${NC}"
else
    echo -e "  ${YELLOW}⚠ AU not found (may not be enabled)${NC}"
fi

if [ ! -e "$VST3_PATH" ] && [ ! -e "$AU_PATH" ]; then
    echo -e "\n${RED}ERROR: No plugins were built!${NC}"
    exit 1
fi

echo

# ============================================================================
# Step 5: Self-sign plugins (ad-hoc signature)
# ============================================================================

echo -e "${CYAN}Step 4: Applying ad-hoc code signature...${NC}"

if [ -e "$VST3_PATH" ]; then
    echo "  Signing: AnalogChannel.vst3"
    codesign --force --deep --sign - "$VST3_PATH"

    # Verify signature
    if codesign --verify --verbose=2 "$VST3_PATH" 2>&1 | grep -q "valid on disk"; then
        echo -e "  ${GREEN}[OK] VST3 signed successfully${NC}"
    else
        echo -e "  ${YELLOW}[WARNING] VST3 signature verification unclear${NC}"
    fi
fi

if [ -e "$AU_PATH" ]; then
    echo "  Signing: AnalogChannel.component"
    codesign --force --deep --sign - "$AU_PATH"

    # Verify signature
    if codesign --verify --verbose=2 "$AU_PATH" 2>&1 | grep -q "valid on disk"; then
        echo -e "  ${GREEN}[OK] AU signed successfully${NC}"
    else
        echo -e "  ${YELLOW}[WARNING] AU signature verification unclear${NC}"
    fi
fi

echo

# ============================================================================
# Step 6: Copy to release folder
# ============================================================================

echo -e "${CYAN}Step 5: Copying plugins to release folder...${NC}"

# Read version from .jucer file
VERSION=$(perl -0777 -ne 'print $1 if /<JUCERPROJECT[^>]*version="([^"]+)"/s' "AnalogChannel.jucer")

if [ -z "$VERSION" ]; then
    echo -e "${RED}ERROR: Could not read version from .jucer file${NC}"
    exit 1
fi

RELEASE_DIR="Builds/AnalogChannel_Release_v${VERSION}"

# Create release directory if it doesn't exist
if [ ! -d "$RELEASE_DIR" ]; then
    mkdir -p "$RELEASE_DIR"
    echo -e "  ${GREEN}Created release directory: $RELEASE_DIR${NC}"
else
    echo -e "  ${YELLOW}Release directory exists: $RELEASE_DIR${NC}"
fi

# Create macOS subdirectory
MACOS_DIR="$RELEASE_DIR/macOS"
mkdir -p "$MACOS_DIR"

# Copy VST3 (macOS)
if [ -e "$VST3_PATH" ]; then
    echo "  Copying: AnalogChannel.vst3 → macOS/"
    rm -rf "$MACOS_DIR/AnalogChannel.vst3"
    cp -R "$VST3_PATH" "$MACOS_DIR/"
    echo -e "  ${GREEN}[OK] VST3 copied to macOS/${NC}"
fi

# Copy AU (macOS)
if [ -e "$AU_PATH" ]; then
    echo "  Copying: AnalogChannel.component → macOS/"
    rm -rf "$MACOS_DIR/AnalogChannel.component"
    cp -R "$AU_PATH" "$MACOS_DIR/"
    echo -e "  ${GREEN}[OK] AU copied to macOS/${NC}"
fi

# Copy installation script (macOS)
INSTALL_SCRIPT="install-macos.sh"
if [ -f "$INSTALL_SCRIPT" ]; then
    echo "  Copying: install-macos.sh → macOS/"
    cp "$INSTALL_SCRIPT" "$MACOS_DIR/"
    chmod +x "$MACOS_DIR/install-macos.sh"
    echo -e "  ${GREEN}[OK] Installation script copied to macOS/${NC}"
else
    echo -e "  ${YELLOW}[WARNING] install-macos.sh not found in project root${NC}"
fi

# Copy README to release root
README_FILE="README.txt"
if [ -f "$README_FILE" ]; then
    echo "  Copying: README.txt → release root"
    cp "$README_FILE" "$RELEASE_DIR/"
    echo -e "  ${GREEN}[OK] README copied to release root${NC}"
else
    echo -e "  ${YELLOW}[WARNING] README.txt not found in project root${NC}"
fi

# Copy LICENSE to release root
LICENSE_FILE="LICENSE.md"
if [ -f "$LICENSE_FILE" ]; then
    echo "  Copying: LICENSE.md → release root"
    cp "$LICENSE_FILE" "$RELEASE_DIR/"
    echo -e "  ${GREEN}[OK] LICENSE copied to release root${NC}"
else
    echo -e "  ${YELLOW}[WARNING] LICENSE.md not found in project root${NC}"
fi

echo

# ============================================================================
# Step 7: Generate PDF Manual
# ============================================================================

echo -e "${CYAN}Step 6: Generating PDF manual...${NC}"

# Check if pandoc is installed
if command -v pandoc &> /dev/null; then
    echo "  Pandoc found, generating PDF..."

    # Create temp directory for PDF generation
    TEMP_DIR="$RELEASE_DIR/.temp_manual"
    mkdir -p "$TEMP_DIR"

    # Copy UserManual.md and images
    if [ -f "Docs/UserManual.md" ]; then
        cp "Docs/UserManual.md" "$TEMP_DIR/"

        if [ -d "Docs/images" ]; then
            cp -R "Docs/images" "$TEMP_DIR/"
        fi

        # Remove TOC (lines 10-47) from copied markdown
        sed -i '' '10,47d' "$TEMP_DIR/UserManual.md"

        # Generate PDF with platform-specific fonts
        cd "$TEMP_DIR"
        pandoc ./UserManual.md -o "$RELEASE_DIR/AnalogChannel_UserManual.pdf" \
            --pdf-engine=xelatex \
            --toc \
            --toc-depth=3 \
            --number-sections \
            -V geometry:margin=2cm \
            -V mainfont="Helvetica Neue" \
            -V monofont="Monaco" \
            -V fontsize=11pt \
            -V linkcolor=blue \
            --highlight-style=tango 2>&1

        if [ $? -eq 0 ] && [ -f "$RELEASE_DIR/AnalogChannel_UserManual.pdf" ]; then
            PDF_SIZE=$(du -h "$RELEASE_DIR/AnalogChannel_UserManual.pdf" | cut -f1)
            echo -e "  ${GREEN}[OK] PDF manual generated: AnalogChannel_UserManual.pdf ($PDF_SIZE)${NC}"
        else
            echo -e "  ${RED}[ERROR] PDF generation failed! Check pandoc output above.${NC}"
            echo -e "  ${YELLOW}Manual must be generated manually.${NC}"
            echo -e "  ${YELLOW}See: InternalDocs/Generate_PDF_manual.md${NC}"
        fi

        cd - > /dev/null

        # Clean up temp directory
        rm -rf "$TEMP_DIR"
    else
        echo -e "  ${RED}[ERROR] Docs/UserManual.md not found!${NC}"
    fi
else
    echo -e "${RED}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${YELLOW}⚠️  WARNING: Pandoc not installed!${NC}"
    echo -e "${RED}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo
    echo -e "${YELLOW}PDF manual will NOT be generated automatically.${NC}"
    echo
    echo -e "${CYAN}To install pandoc:${NC}"
    echo "  macOS:   brew install pandoc basictex"
    echo
    echo -e "${CYAN}You can generate the PDF manually later:${NC}"
    echo "  See: InternalDocs/Generate_PDF_manual.md"
    echo
    echo -e "${YELLOW}Build will continue without PDF...${NC}"
    echo -e "${RED}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
fi

echo

# ============================================================================
# Summary
# ============================================================================

echo -e "${CYAN}============================================${NC}"
echo -e "${GREEN}Build & Sign completed successfully!${NC}"
echo
echo -e "${CYAN}Release folder:${NC}"
echo "  $RELEASE_DIR"
echo
echo -e "${CYAN}Files copied:${NC}"
if [ -e "$VST3_PATH" ]; then
    echo "  ✓ macOS/AnalogChannel.vst3 (Universal: arm64 + x86_64)"
fi
if [ -e "$AU_PATH" ]; then
    echo "  ✓ macOS/AnalogChannel.component (Universal: arm64 + x86_64)"
fi
if [ -f "$MACOS_DIR/install-macos.sh" ]; then
    echo "  ✓ macOS/install-macos.sh (Installation script)"
fi
if [ -f "$RELEASE_DIR/README.txt" ]; then
    echo "  ✓ README.txt (Documentation)"
fi
if [ -f "$RELEASE_DIR/LICENSE.md" ]; then
    echo "  ✓ LICENSE.md (License)"
fi
if [ -f "$RELEASE_DIR/AnalogChannel_UserManual.pdf" ]; then
    echo "  ✓ AnalogChannel_UserManual.pdf (User Manual)"
fi
echo
echo -e "${YELLOW}Note: Plugins are self-signed (ad-hoc signature).${NC}"
echo -e "${YELLOW}Users will need to run install-macos.sh to remove quarantine.${NC}"
echo
echo -e "${CYAN}Next steps:${NC}"
echo "  1. Build Windows version (run build-windows.ps1 on Windows)"
echo "  2. Add documentation to $RELEASE_DIR"
echo "  3. Create ZIP from $RELEASE_DIR"
