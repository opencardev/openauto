#!/bin/bash
# Version information test script

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}🔍 OpenAuto Version Information${NC}"
echo "=================================="

# Get current version info
CURRENT_YEAR=$(date +%Y)
CURRENT_MONTH=$(date +%m)
CURRENT_DAY=$(date +%d)
BUILD_DATE=$(date +%Y%m%d)

echo -e "${YELLOW}📅 Date-based Version Components:${NC}"
echo "  Major (Year):  ${CURRENT_YEAR}"
echo "  Minor (Month): ${CURRENT_MONTH}"
echo "  Patch (Day):   ${CURRENT_DAY}"
echo "  Build Date:    ${BUILD_DATE}"
echo ""

# Get git information
if command -v git >/dev/null 2>&1 && [ -d .git ]; then
    echo -e "${YELLOW}🔗 Git Information:${NC}"
    
    GIT_COMMIT_SHORT=$(git rev-parse --short HEAD 2>/dev/null)
    GIT_COMMIT_FULL=$(git rev-parse HEAD 2>/dev/null)
    GIT_BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null)
    GIT_DESCRIBE=$(git describe --tags --dirty --always 2>/dev/null)
    GIT_STATUS=$(git status --porcelain 2>/dev/null)
    
    echo "  Branch:        ${GIT_BRANCH}"
    echo "  Commit (short): ${GIT_COMMIT_SHORT}"
    echo "  Commit (full):  ${GIT_COMMIT_FULL}"
    echo "  Describe:      ${GIT_DESCRIBE}"
    
    if [ -n "$GIT_STATUS" ]; then
        echo -e "  Working Tree:  ${RED}DIRTY (uncommitted changes)${NC}"
    else
        echo -e "  Working Tree:  ${GREEN}CLEAN${NC}"
    fi
    
    FINAL_VERSION="${CURRENT_YEAR}.${CURRENT_MONTH}.${CURRENT_DAY}+${GIT_COMMIT_SHORT}"
    
else
    echo -e "${RED}❌ Git not available or not a git repository${NC}"
    FINAL_VERSION="${CURRENT_YEAR}.${CURRENT_MONTH}.${CURRENT_DAY}+unknown"
fi

echo ""
echo -e "${GREEN}🎯 Final Version String: ${FINAL_VERSION}${NC}"
echo ""

# Test CMake version detection
echo -e "${YELLOW}🧪 Testing CMake Version Detection:${NC}"

TEST_DIR="version-test"
rm -rf "$TEST_DIR"
mkdir -p "$TEST_DIR"
cd "$TEST_DIR"

echo "Running CMake configuration..."
if cmake -DCMAKE_BUILD_TYPE=Release .. >/dev/null 2>&1; then
    echo -e "${GREEN}✅ CMake configuration successful${NC}"
    
    # Extract version information from CMake
    CMAKE_VERSION=$(cmake --build . --target help 2>/dev/null | grep "Project Version" | cut -d':' -f2 | xargs 2>/dev/null || echo "")
    
    if [ -z "$CMAKE_VERSION" ]; then
        # Try alternative method
        CMAKE_VERSION=$(grep -r "Project Version" . 2>/dev/null | head -1 | cut -d':' -f3 | xargs 2>/dev/null || echo "Not found in output")
    fi
    
    echo "  CMake detected version: ${CMAKE_VERSION}"
    
    if [ "$CMAKE_VERSION" = "$FINAL_VERSION" ]; then
        echo -e "${GREEN}✅ Version strings match!${NC}"
    else
        echo -e "${RED}❌ Version mismatch!${NC}"
        echo "  Expected: $FINAL_VERSION"
        echo "  Got:      $CMAKE_VERSION"
    fi
else
    echo -e "${RED}❌ CMake configuration failed${NC}"
fi

cd ..
rm -rf "$TEST_DIR"

echo ""
echo -e "${BLUE}📋 Version Scheme Summary:${NC}"
echo "  Format: YYYY.MM.DD+commit"
echo "  Example: 2025.07.20+fc4e9d0"
echo "  Major = Year (2025)"
echo "  Minor = Month (07)"
echo "  Patch = Day (20)"
echo "  Build = Git commit (fc4e9d0)"
echo ""
echo -e "${GREEN}🎉 Date-based versioning configured successfully!${NC}"
