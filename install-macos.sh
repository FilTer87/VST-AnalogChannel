#!/bin/bash
# ============================================================================
# AnalogChannel macOS Installation Script
# ============================================================================
# This script installs AnalogChannel plugins and removes quarantine flags
# to allow them to run on macOS without Apple Developer certificate.
#
# Usage:
#   ./install-macos.sh
#
# What it does:
#   1. Removes quarantine flags from downloaded plugins
#   2. Installs VST3 to ~/Library/Audio/Plug-Ins/VST3/
#   3. Installs AU to ~/Library/Audio/Plug-Ins/Components/
#   4. Verifies installation
# ============================================================================

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${CYAN}============================================${NC}"
echo -e "${CYAN}AnalogChannel macOS Installer${NC}"
echo -e "${CYAN}============================================${NC}\n"

# ============================================================================
# Check if running from correct directory
# ============================================================================

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

VST3_FILE="AnalogChannel.vst3"
AU_FILE="AnalogChannel.component"

# ============================================================================
# Find plugin files
# ============================================================================

echo -e "${CYAN}Looking for plugin files...${NC}"

VST3_PATH=""
AU_PATH=""

# Check current directory
if [ -e "$VST3_FILE" ]; then
    VST3_PATH="$SCRIPT_DIR/$VST3_FILE"
    echo -e "  ${GREEN}Found VST3: $VST3_FILE${NC}"
fi

if [ -e "$AU_FILE" ]; then
    AU_PATH="$SCRIPT_DIR/$AU_FILE"
    echo -e "  ${GREEN}Found AU: $AU_FILE${NC}"
fi

# Check if at least one plugin was found
if [ -z "$VST3_PATH" ] && [ -z "$AU_PATH" ]; then
    echo -e "${RED}ERROR: No plugin files found!${NC}"
    echo -e "${YELLOW}Please make sure AnalogChannel.vst3 or AnalogChannel.component${NC}"
    echo -e "${YELLOW}are in the same directory as this script.${NC}"
    exit 1
fi

echo

# ============================================================================
# Remove quarantine flags
# ============================================================================

echo -e "${CYAN}Removing quarantine flags...${NC}"

if [ -n "$VST3_PATH" ]; then
    echo "  Processing: $VST3_FILE"
    xattr -cr "$VST3_PATH"
    echo -e "  ${GREEN}[OK] Quarantine removed from VST3${NC}"
fi

if [ -n "$AU_PATH" ]; then
    echo "  Processing: $AU_FILE"
    xattr -cr "$AU_PATH"
    echo -e "  ${GREEN}[OK] Quarantine removed from AU${NC}"
fi

echo

# ============================================================================
# Create installation directories if needed
# ============================================================================

VST3_DIR="$HOME/Library/Audio/Plug-Ins/VST3"
AU_DIR="$HOME/Library/Audio/Plug-Ins/Components"

echo -e "${CYAN}Checking installation directories...${NC}"

if [ ! -d "$VST3_DIR" ]; then
    echo "  Creating: $VST3_DIR"
    mkdir -p "$VST3_DIR"
fi

if [ ! -d "$AU_DIR" ]; then
    echo "  Creating: $AU_DIR"
    mkdir -p "$AU_DIR"
fi

echo -e "  ${GREEN}[OK] Directories ready${NC}\n"

# ============================================================================
# Install plugins
# ============================================================================

echo -e "${CYAN}Installing plugins...${NC}"

installed_count=0

if [ -n "$VST3_PATH" ]; then
    echo "  Installing VST3 to: $VST3_DIR/"

    # Remove existing installation if present
    if [ -e "$VST3_DIR/$VST3_FILE" ]; then
        echo "    Removing old version..."
        rm -rf "$VST3_DIR/$VST3_FILE"
    fi

    cp -R "$VST3_PATH" "$VST3_DIR/"

    # Remove quarantine from installed copy too
    xattr -cr "$VST3_DIR/$VST3_FILE" 2>/dev/null || true

    echo -e "  ${GREEN}[OK] VST3 installed${NC}"
    ((installed_count++))
fi

if [ -n "$AU_PATH" ]; then
    echo "  Installing AU to: $AU_DIR/"

    # Remove existing installation if present
    if [ -e "$AU_DIR/$AU_FILE" ]; then
        echo "    Removing old version..."
        rm -rf "$AU_DIR/$AU_FILE"
    fi

    cp -R "$AU_PATH" "$AU_DIR/"

    # Remove quarantine from installed copy too
    xattr -cr "$AU_DIR/$AU_FILE" 2>/dev/null || true

    echo -e "  ${GREEN}[OK] AU installed${NC}"
    ((installed_count++))
fi

echo

# ============================================================================
# Clear Audio Unit cache (for AU plugins)
# ============================================================================

if [ -n "$AU_PATH" ]; then
    echo -e "${CYAN}Clearing Audio Unit cache...${NC}"

    AU_CACHE="$HOME/Library/Caches/AudioUnitCache"
    if [ -d "$AU_CACHE" ]; then
        rm -rf "$AU_CACHE"
        echo -e "  ${GREEN}[OK] AU cache cleared${NC}"
        echo -e "  ${YELLOW}Note: Logic Pro and other AU hosts will rescan on next launch${NC}"
    else
        echo -e "  ${YELLOW}No AU cache found (this is normal)${NC}"
    fi

    echo
fi

# ============================================================================
# Verify installation
# ============================================================================

echo -e "${CYAN}Verifying installation...${NC}"

verified_count=0

if [ -n "$VST3_PATH" ]; then
    if [ -e "$VST3_DIR/$VST3_FILE" ]; then
        echo -e "  ${GREEN}✓ VST3 installed at: $VST3_DIR/$VST3_FILE${NC}"
        ((verified_count++))
    else
        echo -e "  ${RED}✗ VST3 installation failed${NC}"
    fi
fi

if [ -n "$AU_PATH" ]; then
    if [ -e "$AU_DIR/$AU_FILE" ]; then
        echo -e "  ${GREEN}✓ AU installed at: $AU_DIR/$AU_FILE${NC}"
        ((verified_count++))
    else
        echo -e "  ${RED}✗ AU installation failed${NC}"
    fi
fi

echo

# ============================================================================
# Summary
# ============================================================================

echo -e "${CYAN}============================================${NC}"
if [ $verified_count -eq $installed_count ] && [ $installed_count -gt 0 ]; then
    echo -e "${GREEN}Installation completed successfully!${NC}"
    echo
    echo -e "${CYAN}Next steps:${NC}"
    echo "  1. Open your DAW (Logic Pro, Ableton, Reaper, etc.)"
    echo "  2. Rescan plugins:"
    echo "     - Logic Pro: Preferences → Audio Units Manager → Reset & Rescan"
    echo "     - Reaper: Preferences → VST → Re-scan"
    echo "     - Ableton: Preferences → Plug-Ins → Rescan"
    echo "  3. Load AnalogChannel on an audio track"
    echo
    echo -e "${YELLOW}Note: On first load, macOS may show a security warning.${NC}"
    echo -e "${YELLOW}If this happens, click 'OK', then go to:${NC}"
    echo -e "${YELLOW}  System Settings → Privacy & Security → Allow anyway${NC}"
else
    echo -e "${RED}Installation completed with errors!${NC}"
    exit 1
fi
