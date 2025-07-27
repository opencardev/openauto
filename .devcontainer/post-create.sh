#!/bin/bash

echo "🚀 Setting up AASDK development environment..."

# Get the target architecture from environment
TARGET_ARCH=${TARGET_ARCH:-amd64}
echo "📦 Target architecture: $TARGET_ARCH"

# Update package list
apt-get update

# Install essential dependencies
echo "📦 Installing dependencies..."
apt-get install -y \
    build-essential \
    cmake \
    git \
    protobuf-compiler \
    libprotobuf-dev \
    libusb-1.0-0-dev \
    libssl-dev \
    libboost-all-dev

# Install cross-compilation toolchains if needed
if [ "$TARGET_ARCH" = "arm64" ]; then
    echo "🔧 Setting up ARM64 cross-compilation..."
    apt-get install -y \
        gcc-aarch64-linux-gnu \
        g++-aarch64-linux-gnu \
        libc6-dev-arm64-cross
    export CC=aarch64-linux-gnu-gcc
    export CXX=aarch64-linux-gnu-g++
elif [ "$TARGET_ARCH" = "armhf" ]; then
    echo "🔧 Setting up ARMHF cross-compilation..."
    apt-get install -y \
        gcc-arm-linux-gnueabihf \
        g++-arm-linux-gnueabihf \
        libc6-dev-armhf-cross
    export CC=arm-linux-gnueabihf-gcc
    export CXX=arm-linux-gnueabihf-g++
else
    echo "🔧 Using native x64 toolchain..."
fi

# Create build directories
echo "📁 Creating build directories..."
mkdir -p build-debug build-release packages protobuf/build

echo "✅ Setup complete!"
echo "💡 You can now build the project using:"
echo "   mkdir -p build-debug && cd build-debug"
echo "   cmake -DCMAKE_BUILD_TYPE=Debug -DTARGET_ARCH=$TARGET_ARCH .."
echo "   make -j\$(nproc)"
echo "   make -j\$(nproc)"
