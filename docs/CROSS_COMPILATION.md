/*
 * Project: OpenAuto
 * This file is part of openauto project.
 * Copyright (C) 2025 OpenCarDev Team
 *
 *  openauto is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  openauto is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with openauto. If not, see <http://www.gnu.org/licenses/>.
 */

# OpenAuto Cross-Compilation Build System

This document describes the cross-compilation build system for OpenAuto, which allows building ARM binaries for Raspberry Pi using GitHub Actions and Docker.

## Overview

The build system supports:
- **ARMhf (32-bit ARM)**: Compatible with Raspberry Pi 1, 2, 3, Zero
- **ARM64 (64-bit ARM)**: Compatible with Raspberry Pi 3, 4, 5 (64-bit OS)
- **Debian Trixie**: Maintains compatibility with modern Raspberry Pi OS
- **Docker-based builds**: Consistent, reproducible build environment
- **GitHub Actions**: Automated builds on push/PR and manual triggers

## GitHub Actions Workflow

### Triggering Builds

#### Automatic Triggers
Builds are automatically triggered on:
- Push to `main`, `develop`, `feature/*`, or `bugfix/*` branches
- Pull requests to `main` or `develop` branches

#### Manual Triggers
Use the GitHub Actions web interface to manually trigger builds with options:
- **Target Architecture**: `armhf`, `arm64`, or `both`
- **Debian Release**: `trixie` (default) or `bookworm`
- **Version**: Custom version string (defaults to `DEV`)
- **Build Tests**: Enable/disable test compilation

### Workflow Steps

1. **Preparation**: Extract version info and create build matrix
2. **Environment Setup**: Configure QEMU for ARM emulation
3. **Docker Build**: Create Debian Trixie container with cross-compilation tools
4. **Cross-Compilation**: Build OpenAuto binaries for target architecture
5. **Artifact Upload**: Store binaries and create release packages

### Artifacts

Each successful build produces:
- Individual binaries (`autoapp-<arch>`, `btservice-<arch>`)
- Compressed release archive (`openauto-<arch>-<version>.tar.gz`)
- SHA256 checksums for verification

## Local Development

### Prerequisites

```bash
# Install Docker and Docker Compose
sudo apt-get update
sudo apt-get install -y docker.io docker-compose-plugin

# Enable QEMU for ARM emulation
sudo apt-get install -y qemu-user-static binfmt-support
```

### Quick Start

```bash
# Quick build for ARMhf
./scripts/quick_build.sh armhf

# Quick build for ARM64
./scripts/quick_build.sh arm64
```

### Using Docker Compose

```bash
# Build for ARMhf
docker compose -f docker-compose.cross.yml --profile armhf up build-armhf

# Build for ARM64
docker compose -f docker-compose.cross.yml --profile arm64 up build-arm64

# Build for both architectures
docker compose -f docker-compose.cross.yml --profile all up

# Development environment (interactive)
docker compose -f docker-compose.cross.yml --profile dev up dev-armhf
```

### Manual Docker Build

```bash
# Create build context
mkdir -p artifacts/armhf

# Build Docker image using the project Dockerfile
docker buildx build \
  --platform linux/arm/v7 \
  --build-arg DEBIAN_RELEASE=trixie \
  --build-arg TARGET_ARCH=armhf \
  --build-arg BUILD_TESTS=false \
  -f Dockerfile.cross \
  -t openauto-builder:armhf \
  .

# Run build
docker run \
  --platform linux/arm/v7 \
  --rm \
  -v "$(pwd):/src" \
  -v "$(pwd)/artifacts/armhf:/output" \
  openauto-builder:armhf
```

## Build Configuration

### Cross-Compilation Setup

The build system uses:
- **CMake Toolchain Files**: Configured for each target architecture
- **Cross-Compilers**: 
  - ARMhf: `arm-linux-gnueabihf-gcc/g++`
  - ARM64: `aarch64-linux-gnu-gcc/g++`
- **Target Libraries**: Architecture-specific packages from Debian repositories

### Dependencies

All required dependencies are installed in the Docker container:
- **Core**: cmake, build-essential, pkg-config
- **Libraries**: Boost, Qt5, protobuf, OpenSSL, ALSA, USB, etc.
- **Cross-tools**: Architecture-specific compilers and toolchains

### CMake Configuration

Key CMake options used:
```cmake
-DCMAKE_BUILD_TYPE=Release
-DCMAKE_TOOLCHAIN_FILE=/toolchain-<arch>.cmake
-DCMAKE_FIND_ROOT_PATH="/usr/<arch-triplet>"
-DNOPI=true
-DBUILD_TESTS=ON/OFF
```

## Architecture Support

### ARMhf (32-bit ARM)
- **Target**: `arm-linux-gnueabihf`
- **Compatible with**: All Raspberry Pi models
- **Use case**: Maximum compatibility, legacy systems

### ARM64 (64-bit ARM)
- **Target**: `aarch64-linux-gnu`
- **Compatible with**: Raspberry Pi 3, 4, 5 with 64-bit OS
- **Use case**: Modern systems, better performance

## Troubleshooting

### Common Issues

#### QEMU Emulation Problems
```bash
# Verify QEMU setup
ls -la /proc/sys/fs/binfmt_misc/
docker run --rm --platform linux/arm64 busybox echo "Test"
```

#### Build Failures
```bash
# Check available disk space
df -h

# View build logs
docker logs <container-id>

# Interactive debugging
docker run -it --platform linux/arm/v7 openauto-builder:armhf /bin/bash
```

#### Missing Dependencies
Dependencies are managed in the Dockerfile. Update package lists if libraries are missing:
```dockerfile
RUN apt-get update && apt-get install -y \
    your-missing-package:${TARGET_ARCH}
```

### Performance Optimization

- **Parallel Jobs**: Limited to 2 on GitHub runners to prevent OOM
- **Disk Space**: Cleanup removes unnecessary packages
- **Build Cache**: Docker layer caching speeds up rebuilds

## Integration with Existing Build System

The cross-compilation system:
- ✅ Uses existing `scripts/build_safe.sh` principles
- ✅ Maintains compatibility with local builds
- ✅ Follows project coding standards and structure
- ✅ Integrates with modern logging system
- ✅ Supports both development and production configurations

## Future Enhancements

Planned improvements:
- **Dependency caching**: Speed up builds by caching compiled dependencies
- **Multi-stage builds**: Reduce final image size
- **Additional platforms**: Support for other ARM variants
- **Integration testing**: Automated testing on target hardware
- **Package generation**: Create .deb packages for easy installation