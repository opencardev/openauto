#!/bin/bash
# Build status checker for OpenAuto CI/CD pipeline

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}🔍 OpenAuto Build Status Checker${NC}"
echo "===================================="

# Check if we're in a git repository
if ! git rev-parse --git-dir > /dev/null 2>&1; then
    echo -e "${RED}❌ Not in a git repository${NC}"
    exit 1
fi

# Get current repository information
REPO_URL=$(git config --get remote.origin.url 2>/dev/null || echo "unknown")
CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "unknown")
CURRENT_COMMIT=$(git rev-parse --short HEAD 2>/dev/null || echo "unknown")
CURRENT_VERSION=$(date +%Y.%m.%d)+${CURRENT_COMMIT}

echo -e "${YELLOW}📋 Repository Information:${NC}"
echo "  Repository: $REPO_URL"
echo "  Branch: $CURRENT_BRANCH"
echo "  Commit: $CURRENT_COMMIT"
echo "  Version: $CURRENT_VERSION"
echo ""

# Check local build environment
echo -e "${YELLOW}🔧 Local Build Environment:${NC}"

# Check for required tools
check_tool() {
    local tool="$1"
    local package="$2"
    
    if command -v "$tool" > /dev/null 2>&1; then
        local version=$($tool --version 2>/dev/null | head -n1 || echo "unknown")
        echo -e "  ✓ $tool: ${GREEN}available${NC} ($version)"
    else
        echo -e "  ❌ $tool: ${RED}missing${NC} (install: $package)"
    fi
}

check_tool "cmake" "cmake"
check_tool "ninja" "ninja-build"
check_tool "gcc" "build-essential"
check_tool "g++" "build-essential"
check_tool "pkg-config" "pkg-config"
check_tool "docker" "docker.io"
check_tool "git" "git"
check_tool "dpkg-deb" "dpkg-dev"
check_tool "lintian" "lintian"

echo ""

# Check for build directories
echo -e "${YELLOW}📁 Build Directories:${NC}"
for dir in build build-amd64 build-arm64 build-armhf build-release build-debug packages; do
    if [ -d "$dir" ]; then
        size=$(du -sh "$dir" 2>/dev/null | cut -f1)
        echo -e "  ✓ $dir: ${GREEN}exists${NC} ($size)"
    else
        echo -e "  ❌ $dir: ${RED}missing${NC}"
    fi
done

echo ""

# Check for recent packages
echo -e "${YELLOW}📦 Recent Packages:${NC}"
if [ -d "packages" ]; then
    package_count=$(find packages -name "*.deb" 2>/dev/null | wc -l)
    if [ "$package_count" -gt 0 ]; then
        echo "  Found $package_count package(s):"
        find packages -name "*.deb" -printf "    %f (%s bytes, %TY-%Tm-%Td %TH:%TM)\n" 2>/dev/null | sort
    else
        echo -e "  ${RED}No packages found${NC}"
    fi
else
    echo -e "  ${RED}No packages directory${NC}"
fi

echo ""

# Check Git status
echo -e "${YELLOW}📝 Git Status:${NC}"
if git diff-index --quiet HEAD -- 2>/dev/null; then
    echo -e "  ✓ Working directory: ${GREEN}clean${NC}"
else
    echo -e "  ⚠ Working directory: ${YELLOW}has changes${NC}"
    echo "    Modified files:"
    git diff --name-only HEAD | sed 's/^/      /'
fi

# Check for unpushed commits
if git rev-parse --verify origin/$CURRENT_BRANCH >/dev/null 2>&1; then
    unpushed_count=$(git rev-list --count origin/$CURRENT_BRANCH..$CURRENT_BRANCH 2>/dev/null || echo "0")
    if [ "$unpushed_count" -gt 0 ]; then
        echo -e "  ⚠ Unpushed commits: ${YELLOW}$unpushed_count${NC}"
    else
        echo -e "  ✓ Branch: ${GREEN}up to date with origin${NC}"
    fi
else
    echo -e "  ⚠ Origin branch: ${YELLOW}not found${NC} (may need to push)"
fi

echo ""

# Check recent tags
echo -e "${YELLOW}🏷️  Recent Tags:${NC}"
recent_tags=$(git tag --sort=-version:refname | head -5)
if [ -n "$recent_tags" ]; then
    echo "$recent_tags" | while read tag; do
        if [ -n "$tag" ]; then
            tag_date=$(git log -1 --format=%ai "$tag" 2>/dev/null || echo "unknown")
            echo "  $tag ($tag_date)"
        fi
    done
else
    echo -e "  ${RED}No tags found${NC}"
fi

echo ""

# Check GitHub Actions status (if we can)
echo -e "${YELLOW}🔄 CI/CD Information:${NC}"
if [ -f ".github/workflows/ci-cd.yml" ]; then
    echo -e "  ✓ CI/CD workflow: ${GREEN}configured${NC}"
    
    # Count workflow files
    workflow_count=$(find .github/workflows -name "*.yml" -o -name "*.yaml" 2>/dev/null | wc -l)
    echo "  Workflows configured: $workflow_count"
    
    # List workflow files
    find .github/workflows -name "*.yml" -o -name "*.yaml" 2>/dev/null | while read workflow; do
        workflow_name=$(basename "$workflow" .yml | basename .yaml)
        echo "    - $workflow_name"
    done
else
    echo -e "  ❌ CI/CD workflow: ${RED}not configured${NC}"
fi

echo ""

# Build recommendations
echo -e "${BLUE}💡 Recommendations:${NC}"

# Check if we should build
if [ -d "packages" ] && [ "$(find packages -name "*.deb" -newer packages 2>/dev/null | wc -l)" -eq 0 ]; then
    if git diff-index --quiet HEAD -- 2>/dev/null; then
        echo -e "  ℹ️  Packages are up to date with current commit"
    else
        echo -e "  🔨 Consider rebuilding packages (working directory has changes)"
    fi
else
    echo -e "  🔨 Consider building packages: ${GREEN}./scripts/build-multiarch.sh -a all${NC}"
fi

# Check if we should create a release
if git tag --points-at HEAD >/dev/null 2>&1; then
    latest_tag=$(git tag --points-at HEAD | head -1)
    echo -e "  🏷️  Current commit is tagged as: $latest_tag"
else
    echo -e "  🏷️  Consider creating a release tag: ${GREEN}git tag v$CURRENT_VERSION${NC}"
fi

# Architecture-specific recommendations
echo ""
echo -e "${BLUE}🏗️  Architecture Build Status:${NC}"
for arch in amd64 arm64 armhf; do
    if [ -f "packages/openauto-modern_*_${arch}.deb" ]; then
        echo -e "  ✓ $arch: ${GREEN}package available${NC}"
    else
        echo -e "  ❌ $arch: ${RED}no package found${NC}"
    fi
done

echo ""
echo -e "${BLUE}📖 Quick Commands:${NC}"
echo "  Build all architectures:     ./scripts/build-multiarch.sh -a all"
echo "  Build specific arch:         ./scripts/build-multiarch.sh -a amd64"
echo "  Build with Docker:           ./scripts/build-multiarch.sh -a arm64 --docker"
echo "  Validate packages:           ./.github/workflows/validate-packages.yml (via GitHub Actions)"
echo "  Create manual release:       GitHub Actions → Manual Release Creation"
echo "  Check CI/CD status:          Visit GitHub Actions tab"

echo ""
echo -e "${GREEN}✅ Build status check completed${NC}"