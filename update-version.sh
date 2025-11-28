#!/bin/bash
# ============================================================================
# AnalogChannel Version Update Script
# ============================================================================
# This script reads the version from AnalogChannel.jucer and updates it
# across all documentation and source files.
#
# Usage:
#   ./update-version.sh
#
# Or manually specify a version:
#   ./update-version.sh 0.5.0
# ============================================================================

set -e

VERSION="$1"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# ============================================================================
# 1. Read version from .jucer if not provided
# ============================================================================

if [ -z "$VERSION" ]; then
    echo -e "${CYAN}Reading version from AnalogChannel.jucer...${NC}"

    JUCER_PATH="AnalogChannel.jucer"
    if [ ! -f "$JUCER_PATH" ]; then
        echo -e "${RED}ERROR: AnalogChannel.jucer not found!${NC}"
        exit 1
    fi

    # Extract version from JUCERPROJECT tag (multi-line aware)
    VERSION=$(perl -0777 -ne 'print $1 if /<JUCERPROJECT[^>]*version="([^"]+)"/s' "$JUCER_PATH")

    if [ -z "$VERSION" ]; then
        echo -e "${RED}ERROR: Could not find version in JUCERPROJECT tag!${NC}"
        exit 1
    fi

    echo -e "${GREEN}Found version: $VERSION${NC}"
fi

echo -e "\n${YELLOW}Updating all files to version: $VERSION${NC}"
echo -e "${YELLOW}============================================${NC}\n"

# ============================================================================
# 2. Update each file
# ============================================================================

updated_count=0
skipped_count=0
error_count=0

# Helper function to update file
update_file() {
    local file_path="$1"
    local pattern="$2"
    local replacement="$3"
    local description="$4"

    echo -e "${CYAN}Processing: $description${NC}"
    echo "  File: $file_path"

    if [ ! -f "$file_path" ]; then
        echo -e "  ${RED}[ERROR] File not found!${NC}"
        ((error_count++))
        echo
        return
    fi

    # Check if pattern exists in file
    if grep -qP "$pattern" "$file_path" 2>/dev/null || perl -ne "exit 0 if /$pattern/" "$file_path"; then
        # Get old value for display
        old_value=$(grep -oP "$pattern" "$file_path" 2>/dev/null || perl -ne "print \$& if /$pattern/" "$file_path" | head -1)

        # Perform replacement (using perl for cross-platform compatibility)
        perl -i -pe "s/$pattern/$replacement/g" "$file_path"

        echo -e "  ${GREEN}[OK] Updated: '$old_value' -> '$replacement'${NC}"
        ((updated_count++))
    else
        echo -e "  ${YELLOW}[WARNING] Pattern not found in file!${NC}"
        echo -e "  ${YELLOW}Pattern: $pattern${NC}"
        ((skipped_count++))
    fi

    echo
}

# Update Docs/UserManual.md - Header (line 4)
update_file \
    "Docs/UserManual.md" \
    '(?<=\*\*Version )[0-9]+\.[0-9]+(\.[0-9]+)?' \
    "$VERSION" \
    "User Manual - Header (line 4)"

# Update Docs/UserManual.md - Plugin Information section (line 1096)
update_file \
    "Docs/UserManual.md" \
    '(?<=- \*\*Version\*\*: )[0-9]+\.[0-9]+(\.[0-9]+)?' \
    "$VERSION" \
    "User Manual - Plugin Information (line 1096)"

# Update Docs/UserManual.md - Footer (line 1258)
update_file \
    "Docs/UserManual.md" \
    '(?<=\*AnalogChannel User Manual v)[0-9]+\.[0-9]+(\.[0-9]+)?(?= \|)' \
    "$VERSION" \
    "User Manual - Footer (line 1258)"

# Update README.txt - Header (line 4)
update_file \
    "README.txt" \
    '(?<=Version )[0-9]+\.[0-9]+(\.[0-9]+)?' \
    "$VERSION" \
    "README.txt - Header (line 4)"

# Update Source/PluginEditor.cpp - About dialog (line 293)
update_file \
    "Source/PluginEditor.cpp" \
    '(?<=AnalogChannel v)[0-9]+\.[0-9]+(\.[0-9]+)?(?=\\n)' \
    "$VERSION" \
    "PluginEditor.cpp - About dialog (line 293)"

# ============================================================================
# 4. Summary
# ============================================================================

echo -e "${YELLOW}============================================${NC}"
echo -e "${YELLOW}Summary:${NC}"
echo -e "  ${CYAN}Version: $VERSION${NC}"
echo -e "  ${GREEN}Updated: $updated_count files${NC}"

if [ $skipped_count -gt 0 ]; then
    echo -e "  ${YELLOW}Skipped: $skipped_count files${NC}"
fi

if [ $error_count -gt 0 ]; then
    echo -e "  ${RED}Errors:  $error_count files${NC}"
    echo -e "\n${RED}Version update completed with errors!${NC}"
    exit 1
else
    echo -e "\n${GREEN}Version update completed successfully!${NC}"
    echo -e "\n${CYAN}Next steps:${NC}"
    echo "  1. Review changes: git diff"
    echo "  2. Open Projucer and save (to regenerate build files)"
    echo "  3. Rebuild (Xcode or Visual Studio)"
    echo "  4. Commit: git add . && git commit -m 'Bump version to $VERSION'"
    echo "  5. Tag: git tag v$VERSION"
    echo "  6. Push: git push && git push --tags"
fi
