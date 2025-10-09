#!/bin/bash
# Safe build script for Raspberry Pi with limited memory

cd "$(dirname "$0")/../build"

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