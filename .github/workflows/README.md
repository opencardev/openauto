# GitHub Actions - Cross-Compilation Workflow

This directory contains the GitHub Actions workflow for cross-compiling OpenAuto for ARM architectures.

## Workflow: `build-cross-compile.yml`

Automatically builds OpenAuto binaries for Raspberry Pi using Docker containers running Debian Trixie.

### Features

- ✅ **Multi-Architecture Support**: ARMhf (32-bit) and ARM64 (64-bit)
- ✅ **Debian Trixie Compatibility**: Maintains Pi OS compatibility
- ✅ **Automated Triggers**: Build on push/PR
- ✅ **Manual Dispatch**: Configurable builds via GitHub UI
- ✅ **Artifact Storage**: Automatic binary uploads
- ✅ **Release Packages**: Compressed archives with checksums

### Triggering Builds

#### Automatic
- Push to: `main`, `develop`, `feature/*`, `bugfix/*`
- Pull requests to: `main`, `develop`

#### Manual
Go to Actions tab → "Cross-Compile Build for ARM" → "Run workflow"

**Options:**
- **Target Architecture**: `armhf`, `arm64`, or `both`
- **Debian Release**: `trixie` (recommended) or `bookworm`
- **Version**: Custom version string (default: `DEV`)
- **Build Tests**: Include test compilation

### Build Process

1. **Environment Setup**: QEMU emulation, Docker Buildx
2. **Cross-Compilation**: Debian container with target libraries
3. **Binary Creation**: `autoapp` and `btservice` executables
4. **Packaging**: Tar archives with SHA256 checksums
5. **Upload**: GitHub artifacts with 30-90 day retention

### Artifacts

Each build produces:
```
openauto-<arch>-<version>/
├── autoapp-<arch>          # Main application binary
├── btservice-<arch>        # Bluetooth service binary
└── <other-binaries>-<arch> # Additional utilities
```

Plus compressed release:
```
openauto-<arch>-<version>.tar.gz
openauto-<arch>-<version>.tar.gz.sha256
```

### Download and Install

1. Go to the Actions tab
2. Click on a successful build
3. Download the artifact for your architecture
4. Transfer to Raspberry Pi:
   ```bash
   scp openauto-armhf-*.tar.gz pi@your-pi:~/
   ssh pi@your-pi
   tar -xzf openauto-armhf-*.tar.gz
   sudo cp autoapp-armhf /usr/local/bin/autoapp
   sudo cp btservice-armhf /usr/local/bin/btservice
   ```

### Architecture Guide

**ARMhf (32-bit)**
- All Raspberry Pi models
- Maximum compatibility
- Use for: Pi 1, 2, Zero, or 32-bit OS

**ARM64 (64-bit)**
- Pi 3, 4, 5 with 64-bit OS
- Better performance
- Use for: Modern Pi with 64-bit Raspberry Pi OS

### Local Testing

Use the included test scripts:
```bash
# Quick build for development
./scripts/quick_build.sh armhf

# Comprehensive test
./scripts/test_cross_compile.sh --arch armhf

# Test ARM64 with tests
./scripts/test_cross_compile.sh --arch arm64 --tests

# Get help
./scripts/test_cross_compile.sh --help
```

Or use Docker Compose:
```bash
# Build for ARMhf
docker compose -f docker-compose.cross.yml --profile armhf up

# Build for both architectures
docker compose -f docker-compose.cross.yml --profile all up
```

Or use the Dockerfile directly:
```bash
# Build image
docker buildx build \
  --platform linux/arm/v7 \
  --build-arg TARGET_ARCH=armhf \
  -f Dockerfile.cross \
  -t openauto-dev:armhf \
  .

# Run build
docker run --platform linux/arm/v7 --rm \
  -v "$(pwd):/src" \
  -v "$(pwd)/artifacts/armhf:/output" \
  openauto-dev:armhf
```

### Troubleshooting

**Build Failures:**
- Check the Actions logs for specific errors
- Verify dependencies in `Dockerfile.cross`
- Test locally with `scripts/test_cross_compile.sh`

**QEMU Issues:**
- Ensure Docker Desktop has experimental features enabled
- Install `qemu-user-static` on runner (handled automatically)

**Missing Binaries:**
- Check build logs for compilation errors
- Verify CMake configuration in Dockerfile
- Test with local Docker build

### Contributing

When modifying the workflow:
1. Test changes locally with Docker Compose
2. Update documentation if adding new features
3. Ensure backward compatibility with existing scripts
4. Follow project coding standards and licensing

For more details, see [`docs/CROSS_COMPILATION.md`](../docs/CROSS_COMPILATION.md).