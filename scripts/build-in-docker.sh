#!/bin/bash
# Docker build helper script - runs inside Docker container

set -e

ARCH="$1"
BUILD_DIR="$2"
BUILD_TYPE="$3"

if [ -z "$ARCH" ] || [ -z "$BUILD_DIR" ] || [ -z "$BUILD_TYPE" ]; then
    echo "Usage: $0 <arch> <build_dir> <build_type>"
    exit 1
fi

echo "Building in Docker for architecture: $ARCH"
echo "Build directory: $BUILD_DIR"
echo "Build type: $BUILD_TYPE"

# Set target based on architecture
case "$ARCH" in
    arm64)
        TARGET="aarch64-linux-gnu"
        CMAKE_PROCESSOR="aarch64"
        ;;
    armhf)
        TARGET="arm-linux-gnueabihf"
        CMAKE_PROCESSOR="arm"
        ;;
    *)
        echo "Unsupported architecture: $ARCH"
        exit 1
        ;;
esac

# Build AASDK first
echo "Building AASDK for $ARCH..."
if [ ! -d "/tmp/aasdk" ]; then
    git clone https://github.com/opencardev/aasdk.git /tmp/aasdk
fi

cd /tmp/aasdk
mkdir -p build && cd build
cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_CXX_STANDARD=17 \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DCMAKE_SYSTEM_NAME=Linux \
    -DCMAKE_SYSTEM_PROCESSOR="$CMAKE_PROCESSOR" \
    -DCMAKE_C_COMPILER="${TARGET}-gcc" \
    -DCMAKE_CXX_COMPILER="${TARGET}-g++" \
    -DTARGET_ARCH="$ARCH" \
    -GNinja

ninja -j$(nproc)
ninja install
ldconfig

# Build OpenAuto
echo "Building OpenAuto for $ARCH..."
cd /workspace

cmake -B "$BUILD_DIR" -S . \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_CXX_STANDARD=17 \
    -DNOPI=ON \
    -DENABLE_MODERN_API=ON \
    -DENABLE_CS_PARAM_CALLS=OFF \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_SYSTEM_NAME=Linux \
    -DCMAKE_SYSTEM_PROCESSOR="$CMAKE_PROCESSOR" \
    -DCMAKE_C_COMPILER="${TARGET}-gcc" \
    -DCMAKE_CXX_COMPILER="${TARGET}-g++" \
    -GNinja

cmake --build "$BUILD_DIR" --parallel $(nproc)

echo "Build completed successfully for $ARCH"
