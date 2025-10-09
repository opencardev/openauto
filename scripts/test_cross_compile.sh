#!/bin/bash
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

# OpenAuto Cross-Compilation Test Script

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Default values
TARGET_ARCH="armhf"
DEBIAN_RELEASE="trixie"
BUILD_TESTS="false"
CLEANUP="true"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --arch)
            TARGET_ARCH="$2"
            shift 2
            ;;
        --release)
            DEBIAN_RELEASE="$2"
            shift 2
            ;;
        --tests)
            BUILD_TESTS="true"
            shift
            ;;
        --no-cleanup)
            CLEANUP="false"
            shift
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --arch ARCH       Target architecture (armhf, arm64) [default: armhf]"
            echo "  --release REL     Debian release (trixie, bookworm) [default: trixie]"
            echo "  --tests           Enable test compilation"
            echo "  --no-cleanup      Don't cleanup Docker images after build"
            echo "  --help            Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0 --arch armhf                    # Build for ARMhf"
            echo "  $0 --arch arm64 --tests            # Build for ARM64 with tests"
            echo "  $0 --arch armhf --release bookworm # Build for ARMhf on Debian Bookworm"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Validate architecture
if [[ "$TARGET_ARCH" != "armhf" && "$TARGET_ARCH" != "arm64" ]]; then
    echo "Error: Invalid architecture '$TARGET_ARCH'. Must be 'armhf' or 'arm64'"
    exit 1
fi

# Determine Docker platform
if [ "$TARGET_ARCH" = "armhf" ]; then
    DOCKER_PLATFORM="linux/arm/v7"
elif [ "$TARGET_ARCH" = "arm64" ]; then
    DOCKER_PLATFORM="linux/arm64"
fi

echo "=========================================="
echo "OpenAuto Cross-Compilation Test"
echo "=========================================="
echo "Target Architecture: $TARGET_ARCH"
echo "Docker Platform: $DOCKER_PLATFORM"
echo "Debian Release: $DEBIAN_RELEASE"
echo "Build Tests: $BUILD_TESTS"
echo "Project Root: $PROJECT_ROOT"
echo "=========================================="

cd "$PROJECT_ROOT"

# Check prerequisites
echo "Checking prerequisites..."

# Check Docker
if ! command -v docker &> /dev/null; then
    echo "Error: Docker is not installed or not in PATH"
    exit 1
fi

# Check Docker Buildx
if ! docker buildx version &> /dev/null; then
    echo "Error: Docker Buildx is not available"
    exit 1
fi

# Check QEMU
if ! docker run --rm --platform "$DOCKER_PLATFORM" busybox:latest echo "QEMU test" &> /dev/null; then
    echo "Warning: QEMU emulation for $DOCKER_PLATFORM may not be working properly"
    echo "Installing qemu-user-static..."
    sudo apt-get update && sudo apt-get install -y qemu-user-static binfmt-support
fi

echo "✅ Prerequisites check complete"

# Create output directory
ARTIFACTS_DIR="$PROJECT_ROOT/artifacts/$TARGET_ARCH"
mkdir -p "$ARTIFACTS_DIR"

# Create Dockerfile if it doesn't exist
DOCKERFILE="$PROJECT_ROOT/Dockerfile.cross"
if [ ! -f "$DOCKERFILE" ]; then
    echo "Error: Dockerfile.cross not found at $DOCKERFILE"
    echo "Please ensure you're running this script from the project root or scripts directory"
    exit 1
fi

# Build Docker image
IMAGE_NAME="openauto-builder:$TARGET_ARCH"
echo "Building Docker image: $IMAGE_NAME"

docker buildx build \
    --platform "$DOCKER_PLATFORM" \
    --build-arg DEBIAN_RELEASE="$DEBIAN_RELEASE" \
    --build-arg TARGET_ARCH="$TARGET_ARCH" \
    --build-arg BUILD_TESTS="$BUILD_TESTS" \
    -f "$DOCKERFILE" \
    -t "$IMAGE_NAME" \
    .

echo "✅ Docker image built successfully"

# Run the build
echo "Starting cross-compilation build..."

docker run \
    --platform "$DOCKER_PLATFORM" \
    --rm \
    -v "$PROJECT_ROOT:/src" \
    -v "$ARTIFACTS_DIR:/output" \
    "$IMAGE_NAME"

echo "✅ Build completed"

# Verify artifacts
echo "Verifying build artifacts..."
if [ -f "$ARTIFACTS_DIR/autoapp-$TARGET_ARCH" ]; then
    echo "✅ autoapp binary: $(file "$ARTIFACTS_DIR/autoapp-$TARGET_ARCH")"
else
    echo "❌ autoapp binary not found"
fi

if [ -f "$ARTIFACTS_DIR/btservice-$TARGET_ARCH" ]; then
    echo "✅ btservice binary: $(file "$ARTIFACTS_DIR/btservice-$TARGET_ARCH")"
else
    echo "❌ btservice binary not found"
fi

# List all artifacts
echo ""
echo "All build artifacts in $ARTIFACTS_DIR:"
ls -la "$ARTIFACTS_DIR/"

# Cleanup
if [ "$CLEANUP" = "true" ]; then
    echo ""
    echo "Cleaning up Docker image..."
    docker rmi "$IMAGE_NAME" || echo "Warning: Could not remove Docker image"
fi

# Create tarball
echo ""
echo "Creating release tarball..."
cd "$ARTIFACTS_DIR"
tar -czf "../openauto-$TARGET_ARCH-local.tar.gz" *
cd "$PROJECT_ROOT"

echo "✅ Release tarball created: artifacts/openauto-$TARGET_ARCH-local.tar.gz"

echo ""
echo "=========================================="
echo "Cross-compilation test completed successfully!"
echo "=========================================="
echo "Binaries are available in: $ARTIFACTS_DIR"
echo "Release tarball: artifacts/openauto-$TARGET_ARCH-local.tar.gz"
echo ""
echo "To transfer to Raspberry Pi:"
echo "  scp artifacts/openauto-$TARGET_ARCH-local.tar.gz pi@your-pi:~/"
echo "  ssh pi@your-pi 'tar -xzf openauto-$TARGET_ARCH-local.tar.gz'"