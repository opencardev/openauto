#!/bin/bash
# * Project: OpenAuto
# * This file is part of openauto project.
# * Copyright (C) 2025 OpenCarDev Team
# *
# *  openauto is free software: you can redistribute it and/or modify
# *  it under the terms of the GNU General Public License as published by
# *  the Free Software Foundation; either version 3 of the License, or
# *  (at your option) any later version.
# *
# *  openauto is distributed in the hope that it will be useful,
# *  but WITHOUT ANY WARRANTY; without even the implied warranty of
# *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# *  GNU General Public License for more details.
# *
# *  You should have received a copy of the GNU General Public License
# *  along with openauto. If not, see <http://www.gnu.org/licenses/>.

set -e

# Script to build OpenAuto consistently across Docker and local environments
# Usage: ./build.sh [release|debug] [--clean] [--package] [--output-dir DIR]

# Default values
NOPI_FLAG="-DNOPI=ON"
CLEAN_BUILD=false
PACKAGE=false
OUTPUT_DIR="/output"
WITH_AASDK=false
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DIR="${SCRIPT_DIR}"

# Auto-detect build type based on git branch
CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "unknown")
if [ "$CURRENT_BRANCH" = "main" ] || [ "$CURRENT_BRANCH" = "master" ]; then
    BUILD_TYPE="release"
else
    BUILD_TYPE="debug"
fi

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        release|Release|RELEASE)
            BUILD_TYPE="release"
            shift
            ;;
        debug|Debug|DEBUG)
            BUILD_TYPE="debug"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --package)
            PACKAGE=true
            shift
            ;;
        --output-dir)
            OUTPUT_DIR="$2"
            shift 2
            ;;
        --with-aasdk)
            WITH_AASDK=true
            shift
            ;;
        --help|-h)
            echo "Usage: $0 [release|debug] [OPTIONS]"
            echo ""
            echo "Build types:"
            echo "  release        Build release version (default)"
            echo "  debug          Build debug version with symbols"
            echo ""
            echo "Options:"
            echo "  --clean        Clean build directory before building"
            echo "  --package      Create DEB packages after building"
            echo "  --output-dir   Directory to copy packages (default: /output)"
            echo "  --with-aasdk   Clone AASDK newdev branch and build/install it"
            echo "  --help         Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0 release --package"
            echo "  $0 debug --clean"
            echo "  $0 release --package --output-dir ./packages"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Handle AASDK cloning and building if requested
if [ "$WITH_AASDK" = true ]; then
    echo ""
    echo "Cloning AASDK newdev branch..."
    if [ -d "aasdk-build" ]; then
        rm -rf "aasdk-build"
    fi
    git clone --branch newdev https://github.com/opencardev/aasdk.git aasdk-build
    cd aasdk-build
    echo "Building and installing AASDK..."
    chmod +x build.sh
    export TARGET_ARCH="$TARGET_ARCH"
    ./build.sh $BUILD_TYPE install
    cd "${SOURCE_DIR}"
    echo "AASDK build and install completed."
fi

# Determine build directory and CMake build type
if [ "$BUILD_TYPE" = "debug" ]; then
    BUILD_DIR="${SOURCE_DIR}/build-debug"
    CMAKE_BUILD_TYPE="Debug"
    CMAKE_CXX_FLAGS="-g3 -O0"
    echo "=== Building OpenAuto (Debug) ==="
else
    BUILD_DIR="${SOURCE_DIR}/build-release"
    CMAKE_BUILD_TYPE="Release"
    CMAKE_CXX_FLAGS=""
    echo "=== Building OpenAuto (Release) ==="
fi

echo "Source directory: ${SOURCE_DIR}"
echo "Build directory: ${BUILD_DIR}"
echo "Build type: ${CMAKE_BUILD_TYPE}"
echo "NOPI: ON (no Pi hardware dependencies)"
echo "Package: ${PACKAGE}"

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo ""
    echo "Cleaning build directory..."
    rm -rf "${BUILD_DIR}"
fi

# Create build directory
mkdir -p "${BUILD_DIR}"

# Detect architecture
TARGET_ARCH=$(dpkg-architecture -qDEB_HOST_ARCH 2>/dev/null || echo "amd64")
echo "Target architecture: ${TARGET_ARCH}"

find_cross_compiler() {
    local prefix="$1"
    local compiler=""
    
    # First try the base name (might be a symlink to latest)
    if command -v "${prefix}gcc" &> /dev/null && [ -x "$(command -v "${prefix}gcc")" ]; then
        compiler="$(command -v "${prefix}gcc")"
    else
        # Find all versioned compilers and pick the latest that actually exists and is executable
        local candidates=()
        for candidate in $(ls /usr/bin/${prefix}gcc-* 2>/dev/null | sort -V); do
            if [ -x "$candidate" ]; then
                candidates+=("$candidate")
            fi
        done
        if [ ${#candidates[@]} -gt 0 ]; then
            compiler="${candidates[-1]}"
        fi
    fi
    
    if [ -n "$compiler" ] && [ -x "$compiler" ]; then
        echo "$compiler"
        return 0
    else
        return 1
    fi
}

setup_cross_compilation() {
    if [ "$TARGET_ARCH" != "amd64" ]; then
        echo "Setting up cross-compilation for ${TARGET_ARCH}..."
        
        case $TARGET_ARCH in
            arm64)
                local c_compiler=$(find_cross_compiler "aarch64-linux-gnu-")
                if [ $? -eq 0 ]; then
                    CMAKE_ARGS+=(-DCMAKE_C_COMPILER="$c_compiler")
                    CMAKE_ARGS+=(-DCMAKE_CXX_COMPILER="${c_compiler/gcc/g++}")
                fi
                ;;
            armhf)
                local c_compiler=$(find_cross_compiler "arm-linux-gnueabihf-")
                if [ $? -eq 0 ]; then
                    CMAKE_ARGS+=(-DCMAKE_C_COMPILER="$c_compiler")
                    CMAKE_ARGS+=(-DCMAKE_CXX_COMPILER="${c_compiler/gcc/g++}")
                fi
                ;;
        esac
    fi
}

setup_cross_compilation

# Compute distro-specific release suffix
if [ -f "${SOURCE_DIR}/scripts/distro_release.sh" ]; then
    DISTRO_DEB_RELEASE=$(bash "${SOURCE_DIR}/scripts/distro_release.sh")
    echo "Distro release suffix: ${DISTRO_DEB_RELEASE}"
else
    DISTRO_DEB_RELEASE=""
    echo "Warning: distro_release.sh not found, using default release suffix"
fi

# Configure CMake
echo ""
echo "Configuring with CMake..."
CMAKE_ARGS=(
    -S "${SOURCE_DIR}"
    -B "${BUILD_DIR}"
    -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
)

if [ -n "$CMAKE_CXX_FLAGS" ]; then
    CMAKE_ARGS+=(-DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS}")
fi

if [ -n "$NOPI_FLAG" ]; then
    CMAKE_ARGS+=("${NOPI_FLAG}")
fi

if [ -n "$DISTRO_DEB_RELEASE" ]; then
    CMAKE_ARGS+=(-DCPACK_DEBIAN_PACKAGE_RELEASE="${DISTRO_DEB_RELEASE}")
fi

CMAKE_ARGS+=(-DCPACK_PROJECT_CONFIG_FILE="${SOURCE_DIR}/cmake_modules/CPackProjectConfig.cmake")

# Run CMake configuration
env DISTRO_DEB_RELEASE="${DISTRO_DEB_RELEASE}" cmake "${CMAKE_ARGS[@]}"

# Build
echo ""
echo "Building..."
NUM_CORES=$(nproc 2>/dev/null || echo 4)
cmake --build "${BUILD_DIR}" -j"${NUM_CORES}"

echo ""
echo "✓ Build completed successfully"

# Package if requested
if [ "$PACKAGE" = true ]; then
    echo ""
    echo "Creating packages..."
    cd "${BUILD_DIR}"
    cpack -G DEB
    cd "${SOURCE_DIR}"
    
    # Copy packages to output directory
    if [ -n "$OUTPUT_DIR" ] && [ "$OUTPUT_DIR" != "${BUILD_DIR}" ]; then
        echo ""
        echo "Copying packages to ${OUTPUT_DIR}..."
        mkdir -p "${OUTPUT_DIR}"
        find "${BUILD_DIR}" -name "*.deb" -exec cp -v {} "${OUTPUT_DIR}/" \;
        echo ""
        echo "Packages in ${OUTPUT_DIR}:"
        ls -lh "${OUTPUT_DIR}"/*.deb 2>/dev/null || echo "No packages found"
    else
        echo ""
        echo "Packages in ${BUILD_DIR}:"
        find "${BUILD_DIR}" -name "*.deb" -ls
    fi
fi

echo ""
echo "=== Build Summary ==="
echo "Build type: ${CMAKE_BUILD_TYPE}"
echo "Build directory: ${BUILD_DIR}"
if [ -f "${BUILD_DIR}/autoapp" ]; then
    echo "Binary: ${BUILD_DIR}/autoapp"
fi
if [ -f "${BUILD_DIR}/btservice" ]; then
    echo "Binary: ${BUILD_DIR}/btservice"
fi

echo ""
echo "Done!"
