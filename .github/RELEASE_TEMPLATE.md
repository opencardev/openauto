# OpenAuto Modern Release Template

Use this template to create consistent release notes for all OpenAuto Modern releases.

## Release Information Template

```markdown
# OpenAuto Modern {VERSION}

**Release Date:** {DATE}  
**Build Date:** {BUILD_DATE}  
**Git Commit:** {GIT_COMMIT}  
**Git Branch:** {GIT_BRANCH}

## 🚀 What's New

{BRIEF_DESCRIPTION_OF_CHANGES}

### ✨ New Features
- {FEATURE_1}
- {FEATURE_2}

### 🐛 Bug Fixes
- {BUG_FIX_1}
- {BUG_FIX_2}

### 🔧 Improvements
- {IMPROVEMENT_1}
- {IMPROVEMENT_2}

### ⚠️ Breaking Changes
- {BREAKING_CHANGE_1} (if any)

## 📦 Package Information

This release provides pre-built packages for multiple architectures:

- **AMD64** - For x86_64 desktop and server systems
- **ARM64** - For 64-bit ARM systems (Raspberry Pi 4/5, etc.)
- **ARMHF** - For 32-bit ARM systems (Raspberry Pi 3, etc.)

Each package includes both release and debug versions.

## 🔧 Installation

### Debian/Ubuntu Systems
```bash
# Download the appropriate package for your architecture
wget https://github.com/opencardev/openauto/releases/download/{TAG}/openauto-modern_{VERSION}_{ARCH}.deb

# Install the package
sudo dpkg -i openauto-modern_{VERSION}_{ARCH}.deb

# Install dependencies if needed
sudo apt-get install -f
```

### Service Management
```bash
# Enable and start the service
sudo systemctl enable openauto
sudo systemctl start openauto

# Check service status
sudo systemctl status openauto
```

## 📋 System Requirements

- **Operating System:** Debian 11+ / Ubuntu 20.04+
- **Architecture:** AMD64, ARM64, or ARMHF
- **RAM:** 2GB minimum, 4GB recommended
- **Storage:** 1GB free space
- **Qt Version:** 5.12+
- **Android Device:** Android 5.0+ with Android Auto support

## 🔧 Dependencies

The packages automatically handle dependencies, but the main requirements include:
- libboost-all-dev
- libprotobuf32
- Qt5 libraries (qtbase5, qtmultimedia5, qtdeclarative5)
- libusb-1.0-0
- libtag1v5
- librtaudio6
- libasound2
- libpulse0

## 📝 Changelog

{DETAILED_CHANGELOG_FROM_GIT}

## 🐛 Known Issues

- {KNOWN_ISSUE_1}
- {KNOWN_ISSUE_2}
- Please check our [issue tracker](https://github.com/opencardev/openauto/issues) for known issues

## 🔗 Links

- **Documentation:** [Build Guide](https://github.com/opencardev/openauto/blob/main/docs/build-guide.md)
- **Configuration:** [Integration Guide](https://github.com/opencardev/openauto/blob/main/docs/integration-guide.md)
- **Support:** [GitHub Issues](https://github.com/opencardev/openauto/issues)
- **Community:** [GitHub Discussions](https://github.com/opencardev/openauto/discussions)

## 🙏 Contributors

Thank you to all contributors who made this release possible!

{CONTRIBUTOR_LIST}

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

**Full Changelog:** https://github.com/opencardev/openauto/compare/{PREVIOUS_TAG}..{CURRENT_TAG}
```

## Release Checklist

Before creating a release, ensure:

- [ ] All tests pass
- [ ] All packages build successfully for all architectures
- [ ] Version number follows semantic versioning (YYYY.MM.DD+commit)
- [ ] Changelog is updated
- [ ] Known issues are documented
- [ ] Breaking changes are clearly marked
- [ ] Installation instructions are updated
- [ ] System requirements are current
- [ ] Dependencies are up to date

## Version Numbering

OpenAuto Modern uses date-based versioning:
- Format: `YYYY.MM.DD+{short_commit_hash}`
- Example: `2025.08.01+a1b2c3d`

For packages, the `+` is replaced with `~` for Debian compatibility:
- Package version: `2025.08.01~a1b2c3d`

## Tag Naming Convention

- Release tags: `v{VERSION}` (e.g., `v2025.08.01+a1b2c3d`)
- Release candidate: `v{VERSION}-rc{N}` (e.g., `v2025.08.01+a1b2c3d-rc1`)
- Beta releases: `v{VERSION}-beta{N}` (e.g., `v2025.08.01+a1b2c3d-beta1`)
- Alpha releases: `v{VERSION}-alpha{N}` (e.g., `v2025.08.01+a1b2c3d-alpha1`)

## Package Naming

Packages follow this naming convention:
- Release: `openauto-modern_{VERSION}_{ARCH}.deb`
- Debug: `openauto-modern-dbg_{VERSION}_{ARCH}.deb`

Where:
- `{VERSION}` is the package version (with `~` instead of `+`)
- `{ARCH}` is one of: `amd64`, `arm64`, `armhf`

## Automation

The CI/CD pipeline automatically:
1. Generates version information from date and git commit
2. Builds packages for all supported architectures
3. Validates package structure and dependencies
4. Creates releases with consistent formatting
5. Uploads packages and checksums
6. Generates release notes from this template

Manual releases can be triggered using the workflow dispatch feature in the GitHub Actions tab.