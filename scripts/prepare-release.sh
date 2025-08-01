#!/bin/bash
# Release preparation script for OpenAuto
# This script helps prepare releases with consistent formatting

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}🚀 OpenAuto Release Preparation Tool${NC}"
echo "======================================"

# Function to get user input with default value
get_input() {
    local prompt="$1"
    local default="$2"
    local var_name="$3"
    
    if [ -n "$default" ]; then
        read -p "$prompt [$default]: " input
        eval "$var_name=\"\${input:-$default}\""
    else
        read -p "$prompt: " input
        eval "$var_name=\"$input\""
    fi
}

# Get release information
echo -e "${YELLOW}📋 Release Information${NC}"
echo "Please provide the following information for the release:"
echo ""

# Get current version info
CURRENT_YEAR=$(date +%Y)
CURRENT_MONTH=$(date +%m)
CURRENT_DAY=$(date +%d)
GIT_COMMIT=$(git rev-parse --short HEAD 2>/dev/null || echo "unknown")
DEFAULT_VERSION="${CURRENT_YEAR}.${CURRENT_MONTH}.${CURRENT_DAY}+${GIT_COMMIT}"

get_input "Release version" "$DEFAULT_VERSION" "RELEASE_VERSION"
get_input "Release type (stable/beta/alpha/rc)" "stable" "RELEASE_TYPE"
get_input "Previous version (for changelog)" "$(git describe --tags --abbrev=0 2>/dev/null || echo 'N/A')" "PREVIOUS_VERSION"

echo ""
echo -e "${YELLOW}📝 Release Content${NC}"
echo "Please provide details about this release:"
echo ""

get_input "Brief description of this release" "" "RELEASE_DESCRIPTION"

# Create release notes file
RELEASE_NOTES_FILE="release-notes-${RELEASE_VERSION}.md"

echo ""
echo -e "${BLUE}📄 Generating release notes template...${NC}"

cat > "$RELEASE_NOTES_FILE" << EOF
# OpenAuto Release $RELEASE_VERSION

**Release Date:** $(date +"%Y-%m-%d")  
**Branch:** \`$(git rev-parse --abbrev-ref HEAD)\`  
**Commit:** \`$(git rev-parse --short HEAD)\`

## 🚀 What's New

$RELEASE_DESCRIPTION

### ✨ New Features
- [TODO: Add new features]

### 🐛 Bug Fixes
- [TODO: Add bug fixes]

### 🔧 Improvements
- [TODO: Add improvements]

### 📚 Documentation
- Updated build and installation guides
- Improved API documentation

## 📝 Changes Since $PREVIOUS_VERSION

EOF

# Add git changelog if previous version exists
if [ "$PREVIOUS_VERSION" != "N/A" ] && git rev-parse "$PREVIOUS_VERSION" >/dev/null 2>&1; then
    echo "Generating changelog from git commits..."
    git log --pretty=format:"- %s (%an)" "$PREVIOUS_VERSION"..HEAD >> "$RELEASE_NOTES_FILE"
else
    echo "- [TODO: Add changelog manually - previous version not found]" >> "$RELEASE_NOTES_FILE"
fi

cat >> "$RELEASE_NOTES_FILE" << EOF

## 🏗️ Build Information

- **Version:** $RELEASE_VERSION
- **Build Date:** $(date +"%Y-%m-%d")
- **Git Commit:** \`$(git rev-parse HEAD)\`
- **Workflow:** Multi-Architecture Build and Release
- **Runner OS:** Linux

## 📦 Available Packages

This release provides pre-built packages for multiple architectures:

### Supported Architectures
- **AMD64** - For x86_64 systems (Intel/AMD 64-bit processors)
- **ARM64** - For AArch64 systems (ARM 64-bit processors, including Raspberry Pi 4+)
- **ARMHF** - For ARM hard-float systems (32-bit ARM processors, including Raspberry Pi 3 and earlier)

### Package Types
- **Release packages** - Optimized for production use
- **Debug packages** - Include debug symbols and additional logging

### Installation

#### Quick Install (Release packages)
\`\`\`bash
# Download and install for your architecture
wget https://github.com/$(git config --get remote.origin.url | sed 's/.*github.com[:/]\([^.]*\).*/\1/')/releases/download/$RELEASE_VERSION/openauto-$RELEASE_VERSION-\$(dpkg --print-architecture).tar.gz
tar -xzf openauto-$RELEASE_VERSION-\$(dpkg --print-architecture).tar.gz
sudo apt install ./openauto-$RELEASE_VERSION-\$(dpkg --print-architecture)/*.deb
\`\`\`

#### Manual Installation
1. Download the appropriate package for your architecture
2. Install using your package manager:
   \`\`\`bash
   sudo apt install ./openauto-modern-*-release-*.deb
   \`\`\`

#### Container Installation
\`\`\`bash
# Pull the latest container image
docker pull ghcr.io/$(git config --get remote.origin.url | sed 's/.*github.com[:/]\([^.]*\).*/\1/'):$RELEASE_VERSION

# Run OpenAuto in container
docker run -d \\
  --name openauto \\
  --privileged \\
  -v /dev:/dev \\
  -v openauto-config:/etc/openauto \\
  -v openauto-data:/var/lib/openauto \\
  -p 8080:8080 \\
  ghcr.io/$(git config --get remote.origin.url | sed 's/.*github.com[:/]\([^.]*\).*/\1/'):$RELEASE_VERSION
\`\`\`

## 🔧 System Requirements

### Minimum Requirements
- **OS:** Ubuntu 20.04+ / Debian 11+ / Raspberry Pi OS
- **RAM:** 1GB minimum, 2GB recommended
- **Storage:** 500MB available space
- **Qt:** 5.12 or later
- **CMake:** 3.16 or later (for building from source)
- **GCC:** 9.0 or later / Clang 10.0 or later (for building from source)

### Hardware Requirements
- **USB Controller:** Compatible with Android Auto
- **Audio:** ALSA or PulseAudio support
- **Display:** Minimum 800x480 resolution
- **Input:** Touchscreen or mouse/keyboard

## 🆕 Migration Guide

### From Previous Versions

[TODO: Add migration instructions if there are breaking changes]

### Configuration Changes

[TODO: Document any configuration file changes]

## 📚 Documentation

- [Build Guide](docs/build-guide.md) - Compile from source
- [Installation Guide](docs/integration-guide.md) - Install and configure
- [API Documentation](docs/api-documentation.md) - Developer reference
- [Modern Architecture](docs/modern-architecture.md) - System architecture

## 🧪 Testing

This release has been tested on:

- ✅ Ubuntu 22.04 LTS (AMD64)
- ✅ Ubuntu 20.04 LTS (AMD64)
- ✅ Raspberry Pi OS (ARM64)
- ✅ Raspberry Pi OS (ARMHF)
- ✅ Debian 11 (AMD64)

### Test Coverage
- Unit tests: [TODO: Add coverage percentage]
- Integration tests: Passed
- Hardware compatibility: Verified on multiple platforms

## 🐛 Known Issues

### Current Limitations
- [TODO: List known issues with workarounds]

### Planned Fixes
- [TODO: List fixes planned for next release]

For a complete list of known issues, please check our [issue tracker](https://github.com/$(git config --get remote.origin.url | sed 's/.*github.com[:/]\([^.]*\).*/\1/')/issues).

## 💡 Getting Help

### Community Support
- [GitHub Discussions](https://github.com/$(git config --get remote.origin.url | sed 's/.*github.com[:/]\([^.]*\).*/\1/')/discussions) - General questions and community chat
- [GitHub Issues](https://github.com/$(git config --get remote.origin.url | sed 's/.*github.com[:/]\([^.]*\).*/\1/')/issues) - Bug reports and feature requests

### Documentation
- [Wiki](https://github.com/$(git config --get remote.origin.url | sed 's/.*github.com[:/]\([^.]*\).*/\1/')/wiki) - Community-maintained documentation

## 🤝 Contributing

We welcome contributions! Please see our [contribution guidelines](docs/code_of_conduct.md) for more information.

## 🙏 Acknowledgments

Special thanks to all contributors who made this release possible:

[TODO: Add contributor acknowledgments]

## 📄 License

This project is licensed under the terms found in the [LICENSE](LICENSE) file.

---

**Full Changelog:** https://github.com/$(git config --get remote.origin.url | sed 's/.*github.com[:/]\([^.]*\).*/\1/')/compare/$PREVIOUS_VERSION...$RELEASE_VERSION  
**Download:** https://github.com/$(git config --get remote.origin.url | sed 's/.*github.com[:/]\([^.]*\).*/\1/')/releases/tag/$RELEASE_VERSION  
**Container Images:** ghcr.io/$(git config --get remote.origin.url | sed 's/.*github.com[:/]\([^.]*\).*/\1/'):$RELEASE_VERSION
EOF

echo -e "${GREEN}✅ Release notes template created: $RELEASE_NOTES_FILE${NC}"
echo ""

# Create tag preparation commands
echo -e "${YELLOW}📋 Next Steps${NC}"
echo "1. Review and edit the release notes file: $RELEASE_NOTES_FILE"
echo "2. Commit any final changes"
echo "3. Create and push the release tag:"
echo ""
echo -e "${BLUE}   git tag -a $RELEASE_VERSION -m \"Release $RELEASE_VERSION\"${NC}"
echo -e "${BLUE}   git push origin $RELEASE_VERSION${NC}"
echo ""
echo "4. The GitHub Actions workflow will automatically:"
echo "   - Build packages for all architectures (amd64, arm64, armhf)"
echo "   - Run validation tests"
echo "   - Create a GitHub release with the packages"
echo "   - Publish container images"
echo ""

# Check if workflow exists
if [ -f ".github/workflows/ci-cd.yml" ]; then
    echo -e "${GREEN}✅ GitHub Actions workflow found${NC}"
else
    echo -e "${RED}❌ GitHub Actions workflow not found${NC}"
    echo "   Please ensure .github/workflows/ci-cd.yml exists"
fi

# Show manual trigger option
echo -e "${YELLOW}💡 Alternative: Manual Trigger${NC}"
echo "You can also trigger the build manually from GitHub Actions:"
echo "1. Go to Actions tab in your GitHub repository"
echo "2. Select 'Multi-Architecture Build and Release' workflow"
echo "3. Click 'Run workflow'"
echo "4. Choose options and run"
echo ""

# Offer to open editor
if command -v code >/dev/null 2>&1; then
    echo -ne "${YELLOW}Would you like to open the release notes in VS Code? (y/N): ${NC}"
    read -r open_editor
    if [[ $open_editor =~ ^[Yy]$ ]]; then
        code "$RELEASE_NOTES_FILE"
    fi
elif command -v nano >/dev/null 2>&1; then
    echo -ne "${YELLOW}Would you like to edit the release notes now? (y/N): ${NC}"
    read -r open_editor
    if [[ $open_editor =~ ^[Yy]$ ]]; then
        nano "$RELEASE_NOTES_FILE"
    fi
fi

echo ""
echo -e "${GREEN}🎉 Release preparation complete!${NC}"
echo -e "${BLUE}Happy releasing! 🚀${NC}"
