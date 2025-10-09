#!/bin/bash
# Safe build script for Raspberry Pi with limited memory

# Get the project root directory (parent of scripts)
PROJECT_ROOT="$(dirname "$0")/.."
BUILD_DIR="$PROJECT_ROOT/build"

# Create build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir -p "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# Check if CMakeCache.txt exists, if not, run cmake
if [ ! -f "CMakeCache.txt" ]; then
    echo "Configuring project with cmake..."
    cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -DNOPI=true -DAASDK_INCLUDE_DIRS=/usr/include/aasdk ..
    if [ $? -ne 0 ]; then
        echo "❌ CMake configuration failed!"
        exit 1
    fi
fi

echo "Building with limited parallelism to avoid OOM issues..."
echo "Available memory: $(free -h | grep Mem | awk '{print $7}')"

# Use at most 2 parallel jobs for 4GB system
JOBS=2

echo "Building with -j${JOBS}..."
make -j${JOBS} "$@"

if [ $? -eq 0 ]; then
    echo "Build completed successfully!"
else
    echo "Build failed. Trying single-threaded build..."
    make -j1 "$@"
fi