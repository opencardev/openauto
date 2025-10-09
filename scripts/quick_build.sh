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

# Quick cross-compilation build using Docker

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Default architecture
ARCH="armhf"

# Parse arguments
case "${1:-}" in
    "armhf"|"arm64")
        ARCH="$1"
        ;;
    "")
        echo "Using default architecture: $ARCH"
        ;;
    *)
        echo "Usage: $0 [armhf|arm64]"
        echo "Builds OpenAuto for the specified ARM architecture using Docker"
        exit 1
        ;;
esac

echo "🚗 OpenAuto Quick Cross-Compilation Build"
echo "=========================================="
echo "Architecture: $ARCH"
echo "Project: $PROJECT_ROOT"

cd "$PROJECT_ROOT"

# Check if Docker is available
if ! command -v docker &> /dev/null; then
    echo "❌ Docker is required but not installed"
    exit 1
fi

# Determine Docker platform
if [ "$ARCH" = "armhf" ]; then
    PLATFORM="linux/arm/v7"
elif [ "$ARCH" = "arm64" ]; then
    PLATFORM="linux/arm64"
fi

echo "Docker platform: $PLATFORM"

# Create output directory
mkdir -p "artifacts/$ARCH"

echo "Building Docker image..."
docker buildx build \
    --platform "$PLATFORM" \
    --build-arg TARGET_ARCH="$ARCH" \
    --build-arg BUILD_TESTS=false \
    -f Dockerfile.cross \
    -t "openauto-dev:$ARCH" \
    .

echo "Starting cross-compilation..."
docker run \
    --platform "$PLATFORM" \
    --rm \
    -v "$PROJECT_ROOT:/src" \
    -v "$PROJECT_ROOT/artifacts/$ARCH:/output" \
    "openauto-dev:$ARCH"

echo ""
echo "✅ Build completed successfully!"
echo "📁 Binaries available in: artifacts/$ARCH/"
echo ""
ls -la "artifacts/$ARCH/"