# OpenAuto CI/CD System

This document describes the comprehensive CI/CD pipeline for building and releasing OpenAuto packages across multiple architectures.

## Overview

The CI/CD system provides:
- **Multi-architecture builds**: AMD64, ARM64, and ARMHF
- **Automated package creation**: Debian packages with proper dependencies
- **Package validation**: Comprehensive testing and linting
- **Automated releases**: Consistent release notes and asset management
- **Manual release control**: On-demand release creation

## Workflows

### CI/CD Pipeline (`ci-cd.yml`) - Unified Workflow

**Purpose:** Comprehensive unified workflow that handles both development testing and production package building

**Triggers:**
- Push to `main`, `develop`, `crankshaft-ng_2025` branches
- Tags starting with `v*` or `release-*`
- Pull requests to `main` or `develop`
- Manual workflow dispatch with full control options

**Manual Workflow Options:**
- **build_type**: Debug or Release builds
- **target_architectures**: Choose specific architectures or 'all'
- **create_release**: Force release creation for non-tag builds
- **release_type**: patch/minor/major for semantic versioning
- **run_tests**: Enable/disable comprehensive testing matrix
- **run_security_scan**: Enable/disable security scanning

**Jobs:**

#### Core Jobs (Always Run)
- **version**: Date-based version generation and build logic determination
- **code-quality**: Static analysis, formatting checks, security scanning

#### Development & Testing Jobs (Conditional)
- **build-test**: Multi-OS and compiler matrix testing (Ubuntu 20.04/22.04, GCC/Clang)
  - Runs when `run_tests` is enabled (default: true)
  - Includes coverage reporting for Debug builds
  - Tests across multiple compiler versions
- **security-scan**: Advanced vulnerability scanning with Trivy
  - Runs when `run_security_scan` is enabled (default: true)

#### Production Jobs (Conditional)
- **build-packages**: Multi-architecture Debian package creation (AMD64/ARM64/ARMHF)
  - Runs for: tagged releases, pushes to main branches, or manual with `create_release`
  - Uses native compilation for AMD64, Docker cross-compilation for ARM
- **validate-packages**: Package integrity and installation testing
- **create-release**: Comprehensive release creation with assets and notes
  - Runs for: tagged releases or manual with `create_release` enabled
- **post-release**: Post-release cleanup and notifications

#### Smart Job Execution
The workflow intelligently decides which jobs to run based on:
- **Pull Requests**: Code quality + testing matrix only
- **Branch Pushes**: Full pipeline with package creation
- **Tagged Releases**: Full pipeline + release creation
- **Manual Runs**: User-controlled via input parameters

### Package Validation (`validate-packages.yml`)

**Purpose:** Standalone package testing and validation

**Use Case:** Independent package testing without full CI/CD pipeline

### Manual Release (`manual-release.yml`)

**Purpose:** Legacy manual release workflow

**Status:** ⚠️ **Deprecated** - Use the unified workflow's manual dispatch instead

## Workflow Migration

### What Changed

1. **Consolidated Workflows**: Combined `ci.yml` and `ci-cd.yml` into a single `ci-cd.yml`
2. **Smart Execution**: Jobs run conditionally based on context and user inputs
3. **Enhanced Manual Control**: Comprehensive manual workflow dispatch options
4. **Improved Efficiency**: Eliminates duplicate jobs and reduces CI resource usage
5. **Better UX**: Single workflow to understand and maintain

### Backup Files

The original workflows have been preserved as:
- `ci.yml.backup` - Original development/testing workflow
- `ci-cd.yml.backup` - Original production package workflow

These can be restored if needed or used for reference.

## Workflow Selection Guide

| Use Case                        | How to Trigger                                       | Jobs That Run                                            |
| ------------------------------- | ---------------------------------------------------- | -------------------------------------------------------- |
| **Development/PR Testing**      | Create Pull Request                                  | `version`, `code-quality`, `build-test`, `security-scan` |
| **Branch Development**          | Push to `main`/`develop`                             | All jobs including package creation                      |
| **Production Release**          | Push tag `v*` or `release-*`                         | Full pipeline + release creation                         |
| **Manual Testing**              | Manual dispatch with `run_tests=true`                | Development jobs only                                    |
| **Manual Release**              | Manual dispatch with `create_release=true`           | Full pipeline + release                                  |
| **Architecture-Specific Build** | Manual dispatch with specific `target_architectures` | Targeted package building                                |

## Version Scheme

OpenAuto uses **date-based versioning**:

```
Format: YYYY.MM.DD+{short_commit_hash}
Example: 2025.08.01+a1b2c3d
```

For Debian packages, `+` is replaced with `~` for compatibility:
```
Package Version: 2025.08.01~a1b2c3d
```

## Package Types

### Main Package: `openauto-modern`
- Contains the main application and libraries
- System service files
- Configuration files
- udev rules

### Debug Package: `openauto-modern-dbg`
- Debug symbols for troubleshooting
- Automatically generated from main package
- Links to main package for installation

## Supported Architectures

| Architecture | Description    | Target Systems                 |
| ------------ | -------------- | ------------------------------ |
| **amd64**    | x86_64         | Desktop PCs, Intel/AMD servers |
| **arm64**    | AArch64        | Raspberry Pi 4/5, ARM64 SBCs   |
| **armhf**    | ARM hard float | Raspberry Pi 3, ARM32 SBCs     |

## Build Scripts

### `scripts/build-multiarch.sh`

Comprehensive build script supporting:
- Multiple architectures
- Native and cross-compilation
- Docker-based builds
- Package validation
- Debug package creation

**Usage:**
```bash
# Build for all architectures
./scripts/build-multiarch.sh -a all

# Build specific architectures
./scripts/build-multiarch.sh -a amd64 -a arm64

# Use Docker for cross-compilation
./scripts/build-multiarch.sh -a arm64 --docker

# Debug build without validation
./scripts/build-multiarch.sh -t Debug --no-validate
```

**Options:**
- `-a, --arch`: Target architecture (amd64, arm64, armhf, all)
- `-t, --type`: Build type (Release, Debug)
- `-o, --output`: Output directory
- `--docker`: Use Docker for cross-compilation
- `--no-clean`: Don't clean build directories
- `--no-validate`: Skip package validation
- `--no-debug`: Don't create debug packages

### `scripts/build-in-docker.sh`

Helper script for Docker-based cross-compilation. Automatically called by the main build script when using `--docker` option.

## Release Process

### Automatic Releases

1. **Tag Creation**: Push a tag starting with `v` or `release-`
   ```bash
   git tag v2025.08.01+a1b2c3d
   git push origin v2025.08.01+a1b2c3d
   ```

2. **Pipeline Execution**: 
   - Builds packages for all architectures
   - Validates packages
   - Creates release with generated notes

3. **Asset Upload**:
   - Debian packages (`.deb`)
   - Debug packages (`-dbg.deb`)
   - Checksums (`checksums.sha256`)
   - Package inventory (`package-inventory.md`)

### Manual Releases

1. **Trigger Workflow**: Go to Actions → Manual Release Creation
2. **Configure Options**:
   - Release type (patch/minor/major)
   - Target architectures
   - Pre-release/draft status
   - Custom release notes

3. **Monitor Progress**: Watch the workflow execute all build and validation steps

## Release Template

Releases follow a consistent format defined in `.github/RELEASE_TEMPLATE.md`:

- **Version Information**: Date, commit, branch
- **What's New**: Features, fixes, improvements
- **Installation Instructions**: Step-by-step guide
- **System Requirements**: OS, dependencies, hardware
- **Changelog**: Git commit history
- **Known Issues**: Current limitations
- **Links**: Documentation, support, community

## Package Dependencies

### Runtime Dependencies
- `libboost-all-dev`
- `libprotobuf32`
- `qtbase5-dev`, `qtmultimedia5-dev`, `qtdeclarative5-dev`
- `qtquickcontrols2-5-dev`
- `libusb-1.0-0`
- `libtag1v5`
- `librtaudio6`
- `libasound2`
- `libpulse0`

### Build Dependencies
- `build-essential`
- `cmake` (≥3.5.1)
- `ninja-build`
- `pkg-config`
- Development headers for runtime dependencies
- Cross-compilation toolchains (for ARM builds)

### AASDK (Android Auto SDK)
- **Repository**: https://github.com/opencardev/aasdk
- **Purpose**: Android Auto protocol implementation
- **Build**: Automatically built from source during CI/CD
- **Configuration**: Uses OpenCarDev's enhanced AASDK with TARGET_ARCH support
- **Integration**: Linked statically into OpenAuto packages

## Quality Assurance

### Code Quality Checks
- **cppcheck**: Static analysis
- **clang-format**: Code formatting
- **Pre-commit hooks**: Automated checks

### Package Validation
- **Structure validation**: Control files, required files
- **Dependency checking**: Proper dependency declaration
- **Installation testing**: Multi-version Ubuntu testing
- **Lintian compliance**: Debian package standards
- **Performance analysis**: Binary size, security features

### Security
- **Binary stripping**: Release binaries are stripped
- **Security features**: Stack protection, BIND_NOW
- **Dependency scanning**: Vulnerability detection

## Troubleshooting

### Unified Workflow Issues

1. **Jobs Not Running as Expected**:
   ```bash
   # Check workflow conditions in the GitHub Actions tab
   # For manual runs, verify input parameters
   # For PRs: only code-quality and build-test jobs run
   # For pushes: full pipeline runs
   # For tags: full pipeline + release creation
   ```

2. **Manual Workflow Dispatch**:
   ```bash
   # Go to Actions → CI/CD Pipeline - Unified → Run workflow
   # Set appropriate parameters:
   # - target_architectures: "amd64,arm64" or "all"
   # - create_release: true (for manual releases)
   # - run_tests: false (to skip testing matrix for faster builds)
   ```

3. **Package Building Issues**:
   ```bash
   # Check the build-packages job logs
   # Verify target_architectures input matches desired builds
   # For ARM builds, check Docker/QEMU setup
   ```

### Build Failures

1. **Dependency Issues**:
   ```bash
   # Update package lists
   sudo apt-get update
   
   # Install missing dependencies
   sudo apt-get install -f
   ```

2. **Cross-compilation Issues**:
   ```bash
   # Add target architecture
   sudo dpkg --add-architecture arm64
   sudo apt-get update
   
   # Install cross-compilation tools
   sudo apt-get install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
   ```

3. **Docker Issues**:
   ```bash
   # Ensure Docker is running
   sudo systemctl start docker
   
   # Enable QEMU for cross-platform builds
   docker run --rm --privileged multiarch/qemu-user-static --reset -p yes
   ```

### Package Issues

1. **Installation Failures**:
   ```bash
   # Check package integrity
   dpkg-deb --info package.deb
   
   # Force dependency installation
   sudo apt-get install -f
   ```

2. **Missing Files**:
   ```bash
   # Check package contents
   dpkg-deb --contents package.deb
   
   # Extract and inspect
   dpkg-deb --extract package.deb extracted/
   ```

## Development Workflow

### Local Development
1. Use `scripts/build-multiarch.sh` for local builds
2. Test packages on target systems
3. Validate using the validation workflow

### Pull Requests
1. CI runs automatically on PR creation
2. All architectures are built and tested
3. Package validation must pass

### Release Preparation
1. Update documentation
2. Test on multiple platforms
3. Create release using manual workflow or git tags

## Monitoring and Maintenance

### Artifact Management
- Packages are retained for 30 days
- Old artifacts are automatically cleaned up
- Release assets are permanent

### Performance Monitoring
- Build times are tracked
- Package sizes are monitored
- Performance regressions are detected

### Security Updates
- Dependencies are regularly updated
- Security scans are performed
- Vulnerabilities are addressed promptly

## Contributing

To contribute to the CI/CD system:

1. Test changes on your fork first
2. Ensure all architectures build successfully
3. Update documentation for any new features
4. Follow the existing workflow patterns

For more information, see:
- [Build Guide](../docs/build-guide.md)
- [Integration Guide](../docs/integration-guide.md)
- [GitHub Issues](https://github.com/opencardev/openauto/issues)
