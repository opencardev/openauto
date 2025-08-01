# Multi-Architecture CI/CD Pipeline

This directory contains the GitHub Actions workflows and scripts for building, testing, and releasing OpenAuto packages across multiple architectures.

## 🏗️ Pipeline Overview

The CI/CD pipeline automatically builds OpenAuto packages for:
- **AMD64** - x86_64 systems (Intel/AMD 64-bit processors)
- **ARM64** - AArch64 systems (ARM 64-bit processors, Raspberry Pi 4+)
- **ARMHF** - ARM hard-float systems (32-bit ARM, Raspberry Pi 3 and earlier)

### Build Types
- **Release** - Optimized production builds
- **Debug** - Development builds with debug symbols and sanitizers

## 📁 Files Structure

```
.github/
├── workflows/
│   ├── ci-cd.yml              # Main multi-architecture build pipeline
│   ├── ci.yml                 # Legacy CI pipeline (can be removed)
│   └── validate-packages.yml  # Package validation workflow
├── RELEASE_TEMPLATE.md        # Template for consistent release notes
└── README.md                  # This file

scripts/
├── prepare-release.sh         # Release preparation helper
└── build-status.sh           # Build status dashboard
```

## 🚀 Workflows

### 1. Multi-Architecture Build and Release (`ci-cd.yml`)

**Triggers:**
- Push to `main`, `develop`, or `crankshaft-ng_2025` branches
- Git tags matching `v*` or version patterns
- Pull requests to `main` or `develop`
- Manual workflow dispatch

**Jobs:**
1. **Prepare** - Determines build matrix and version information
2. **Build** - Cross-compiles for each architecture and build type
3. **Test** - Validates packages on target platforms
4. **Release** - Creates GitHub releases with comprehensive notes
5. **Publish Container** - Builds and publishes multi-arch container images
6. **Cleanup** - Removes old artifacts

### 2. Package Validation (`validate-packages.yml`)

**Purpose:** Reusable workflow for validating package structure and installation

**Features:**
- Package metadata verification
- Installation testing in containers
- File permission checks
- Validation report generation

## 🎯 Usage

### Automated Releases (Recommended)

1. **Tag-based releases:**
   ```bash
   git tag v2025.08.01
   git push origin v2025.08.01
   ```

2. **Using the preparation script:**
   ```bash
   ./scripts/prepare-release.sh
   # Follow the prompts, then push the created tag
   ```

### Manual Builds

1. **Via GitHub Actions UI:**
   - Go to Actions tab → "Multi-Architecture Build and Release"
   - Click "Run workflow"
   - Select architecture and build type

2. **Via GitHub CLI:**
   ```bash
   gh workflow run "Multi-Architecture Build and Release" \
     --field architecture=all \
     --field build_type=both
   ```

### Monitoring Builds

Use the status dashboard:
```bash
./scripts/build-status.sh
```

## 📦 Release Process

### Automatic Release Creation

When a tag is pushed, the pipeline automatically:

1. **Builds packages** for all architectures
2. **Runs validation tests** on each package
3. **Creates release assets:**
   - Individual architecture archives (`.tar.gz` and `.zip`)
   - Combined archive with all architectures
   - Package validation reports
4. **Generates comprehensive release notes** using the template
5. **Publishes to GitHub Releases**
6. **Builds and pushes container images**

### Release Notes Template

Release notes are generated using `.github/RELEASE_TEMPLATE.md` with:
- Automatic version and date information
- Git changelog since last tag
- Package installation instructions
- System requirements
- Documentation links
- Known issues section

### Manual Release Preparation

Use the preparation script for consistent releases:

```bash
./scripts/prepare-release.sh
```

This script:
- Prompts for release information
- Generates formatted release notes
- Provides git commands for tagging
- Shows next steps

## 🔧 Configuration

### Environment Variables

The pipeline uses these environment variables:

- `REGISTRY` - Container registry (default: `ghcr.io`)
- `IMAGE_NAME` - Container image name (auto: `${{ github.repository }}`)

### Workflow Inputs

**Manual workflow dispatch supports:**
- `architecture` - Target architecture (`all`, `amd64`, `arm64`, `armhf`)
- `build_type` - Build type (`both`, `release`, `debug`)
- `create_release` - Force release creation

### Customization

#### Adding New Architectures

1. Update the architecture matrix in `ci-cd.yml`:
   ```yaml
   architectures: ["amd64", "arm64", "armhf", "new-arch"]
   ```

2. Add Docker platform mapping in build script:
   ```bash
   case "$ARCH" in
     new-arch) DOCKER_PLATFORM="linux/new-arch" ;;
   esac
   ```

#### Modifying Build Options

Edit the build script section in `ci-cd.yml`:
```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=${BUILD_TYPE^} \
  -DNOPI=$([ "$ARCH" = "amd64" ] && echo "ON" || echo "OFF") \
  -DYOUR_NEW_OPTION=ON
```

## 🧪 Testing

### Package Validation

Each built package is automatically tested:
- Structure validation (metadata, files, permissions)
- Installation testing in clean containers
- Basic functionality verification

### Manual Testing

Test packages locally:
```bash
# Download packages from release
wget https://github.com/owner/repo/releases/download/v1.0.0/openauto-1.0.0-amd64.tar.gz

# Extract and test
tar -xzf openauto-1.0.0-amd64.tar.gz
sudo apt install ./openauto-1.0.0-amd64/*.deb
```

## 📊 Monitoring

### Build Status

Check current build status:
```bash
./scripts/build-status.sh
```

### GitHub Actions

Monitor builds at:
`https://github.com/owner/repo/actions`

### Container Images

Images are published to:
`ghcr.io/owner/repo:version`

Available tags:
- `latest` - Latest stable release
- `main` - Latest main branch build
- `develop` - Latest develop branch build
- `v1.0.0` - Specific version tags

## 🚨 Troubleshooting

### Common Issues

#### Build Failures

1. **Dependency issues:**
   - Check if new dependencies are added to Dockerfile
   - Verify package availability for all architectures

2. **Cross-compilation errors:**
   - Ensure QEMU setup is working
   - Check architecture-specific flags

3. **Package creation failures:**
   - Verify CMake package configuration
   - Check file permissions and paths

#### Release Issues

1. **Missing packages:**
   - Check if all architecture builds completed
   - Verify artifact upload/download

2. **Release notes errors:**
   - Check template syntax
   - Verify git tag/commit references

### Debug Commands

```bash
# Check workflow runs
gh run list --limit 20

# View specific run logs
gh run view RUN_ID --log

# Check repository secrets
gh secret list

# Trigger test build
gh workflow run "Multi-Architecture Build and Release" \
  --field architecture=amd64 \
  --field build_type=debug
```

## 🔒 Security

### Secrets Required

- `GITHUB_TOKEN` - Automatically provided
- Additional secrets for external services (if needed)

### Permissions

The workflows require:
- `contents: write` - For creating releases
- `packages: write` - For publishing container images
- `security-events: write` - For security scanning

## 📚 Documentation

### Related Documentation

- [Build Guide](../docs/build-guide.md) - Manual build instructions
- [Integration Guide](../docs/integration-guide.md) - Installation and setup
- [Modern Architecture](../docs/modern-architecture.md) - System design

### External Resources

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [Docker Buildx Documentation](https://docs.docker.com/buildx/)
- [QEMU User Emulation](https://qemu.readthedocs.io/en/latest/user/main.html)

## 🤝 Contributing

### Workflow Changes

1. Test changes in a fork first
2. Update documentation
3. Submit pull request with description

### Adding Features

1. Update workflow files
2. Add/update scripts as needed
3. Update this README
4. Test with various scenarios

---

## 📝 Change Log

### Recent Updates

- **2025-08-01**: Initial multi-architecture pipeline
- Added comprehensive release notes template
- Implemented package validation
- Created helper scripts for release management

### Planned Improvements

- [ ] Add performance benchmarking
- [ ] Implement security scanning
- [ ] Add notification system
- [ ] Create deployment automation
