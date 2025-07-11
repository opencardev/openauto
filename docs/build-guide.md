# OpenAuto Build Guide

## Overview

This guide provides comprehensive instructions for building OpenAuto with the modern architecture, including the new logger, event bus, state machine, and REST API components.

## Prerequisites

### System Requirements

**Linux (Recommended):**
- Ubuntu 20.04 LTS or newer
- Debian 11 or newer
- Arch Linux (latest)

**Windows:**
- Windows 10/11 with WSL2
- Visual Studio 2019/2022 with C++ tools
- MSYS2/MinGW-w64 (alternative)

**Hardware:**
- 4GB RAM minimum (8GB recommended)
- 2GB disk space
- Multi-core processor recommended

### Required Dependencies

#### Core Dependencies
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    pkg-config \
    git \
    libboost-all-dev \
    libprotobuf-dev \
    protobuf-compiler \
    libssl-dev \
    libusb-1.0-0-dev \
    libudev-dev

# Arch Linux
sudo pacman -S \
    base-devel \
    cmake \
    pkgconf \
    git \
    boost \
    protobuf \
    openssl \
    libusb \
    systemd
```

#### Qt Dependencies
```bash
# Ubuntu/Debian
sudo apt install -y \
    qtbase5-dev \
    qtmultimedia5-dev \
    qtconnectivity5-dev \
    libqt5multimedia5-plugins \
    libqt5bluetooth5 \
    libqt5bluetooth5-bin

# Arch Linux
sudo pacman -S \
    qt5-base \
    qt5-multimedia \
    qt5-connectivity
```

#### Audio Dependencies
```bash
# Ubuntu/Debian
sudo apt install -y \
    libasound2-dev \
    libpulse-dev \
    librtaudio-dev \
    libtag1-dev

# Arch Linux
sudo pacman -S \
    alsa-lib \
    libpulse \
    rtaudio \
    taglib
```

#### Video Dependencies
```bash
# Ubuntu/Debian
sudo apt install -y \
    libomxil-bellagio-dev \
    libavformat-dev \
    libavcodec-dev \
    libavutil-dev \
    libswscale-dev

# For Raspberry Pi
sudo apt install -y \
    libraspberrypi-dev
```

#### Modern Architecture Dependencies
```bash
# Ubuntu/Debian
sudo apt install -y \
    nlohmann-json3-dev \
    libhttplib-dev \
    libevent-dev

# If packages not available, they will be built from source
```

### Optional Dependencies

#### GPS Support
```bash
sudo apt install -y \
    libgps-dev \
    gpsd \
    gpsd-clients
```

#### Bluetooth Support
```bash
sudo apt install -y \
    libbluetooth-dev \
    bluez \
    bluez-tools
```

## Build Configuration

### CMake Options

```bash
# Core options
-DENABLE_MODERN_API=ON          # Enable modern architecture (required)
-DENABLE_LOGGER_DEMO=ON         # Build logger demonstration
-DENABLE_REST_API=ON            # Enable REST API server
-DENABLE_EVENT_BUS=ON           # Enable modern event bus
-DENABLE_STATE_MACHINE=ON       # Enable state machine

# Platform-specific options
-DRPI_BUILD=ON                  # Raspberry Pi build
-DENABLE_OMX=ON                 # Hardware video acceleration
-DENABLE_GSTREAMER=ON           # GStreamer video support

# Feature options
-DENABLE_GPS=ON                 # GPS sensor support
-DENABLE_BLUETOOTH_SERVICE=ON   # Bluetooth service
-DENABLE_WIFI_PROJECTION=ON     # WiFi projection support

# Debug options
-DCMAKE_BUILD_TYPE=Debug        # Debug build
-DENABLE_SANITIZERS=ON          # Memory/address sanitizers
-DENABLE_COVERAGE=ON            # Code coverage analysis
```

## Build Instructions

### 1. Clone Repository

```bash
git clone https://github.com/opencardev/openauto.git
cd openauto
```

### 2. Initialize Submodules

```bash
git submodule update --init --recursive
```

### 3. Create Build Directory

```bash
mkdir build
cd build
```

### 4. Configure Build

#### Standard Build
```bash
cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_MODERN_API=ON \
    -DENABLE_REST_API=ON \
    -DENABLE_EVENT_BUS=ON \
    -DENABLE_STATE_MACHINE=ON \
    ..
```

#### Development Build
```bash
cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_MODERN_API=ON \
    -DENABLE_LOGGER_DEMO=ON \
    -DENABLE_REST_API=ON \
    -DENABLE_EVENT_BUS=ON \
    -DENABLE_STATE_MACHINE=ON \
    -DENABLE_SANITIZERS=ON \
    ..
```

#### Raspberry Pi Build
```bash
cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DRPI_BUILD=ON \
    -DENABLE_MODERN_API=ON \
    -DENABLE_OMX=ON \
    -DENABLE_GPS=ON \
    -DENABLE_BLUETOOTH_SERVICE=ON \
    ..
```

### 5. Build

```bash
# Parallel build (recommended)
cmake --build . --parallel $(nproc)

# Or use make directly
make -j$(nproc)

# Verbose build for debugging
cmake --build . --verbose
```

### 6. Install (Optional)

```bash
sudo cmake --install .
```

## Cross-Compilation

### Raspberry Pi Cross-Compilation

#### Setup Cross-Compiler
```bash
# Install cross-compiler
sudo apt install -y gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf

# Create toolchain file
cat > rpi-toolchain.cmake << 'EOF'
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

set(CMAKE_FIND_ROOT_PATH /usr/arm-linux-gnueabihf)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
EOF
```

#### Configure and Build
```bash
cmake \
    -DCMAKE_TOOLCHAIN_FILE=../rpi-toolchain.cmake \
    -DRPI_BUILD=ON \
    -DENABLE_MODERN_API=ON \
    ..

make -j$(nproc)
```

## Docker Build

### Create Dockerfile
```dockerfile
FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential cmake pkg-config git \
    libboost-all-dev libprotobuf-dev protobuf-compiler \
    libssl-dev libusb-1.0-0-dev libudev-dev \
    qtbase5-dev qtmultimedia5-dev qtconnectivity5-dev \
    libasound2-dev libpulse-dev librtaudio-dev \
    nlohmann-json3-dev && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

# Build
RUN mkdir build && cd build && \
    cmake -DENABLE_MODERN_API=ON .. && \
    make -j$(nproc)

EXPOSE 8080
CMD ["./build/autoapp"]
```

### Build Docker Image
```bash
docker build -t openauto:latest .
docker run -p 8080:8080 --device=/dev/bus/usb openauto:latest
```

## Testing the Build

### 1. Basic Functionality Test
```bash
# Test autoapp startup
./autoapp --help

# Test logger demo
./logger_demo

# Test btservice
./btservice --test
```

### 2. Modern Architecture Test
```bash
# Test REST API (if enabled)
curl http://localhost:8080/api/v1/status

# Test event bus
./autoapp --test-events

# Test configuration
./autoapp --dump-config
```

### 3. Hardware Tests
```bash
# Test USB devices
lsusb
./autoapp --list-devices

# Test audio
aplay /usr/share/sounds/alsa/Front_Left.wav
./autoapp --test-audio

# Test video (if available)
./autoapp --test-video
```

## Build Optimization

### Performance Builds

#### Release with Link-Time Optimization
```bash
cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON \
    -DENABLE_MODERN_API=ON \
    ..
```

#### Profile-Guided Optimization
```bash
# First build with profiling
cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-fprofile-generate" \
    -DENABLE_MODERN_API=ON \
    ..
make -j$(nproc)

# Run typical workload
./autoapp --profile-run

# Rebuild with profile data
cmake \
    -DCMAKE_CXX_FLAGS="-fprofile-use" \
    ..
make -j$(nproc)
```

### Memory-Optimized Builds
```bash
cmake \
    -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DENABLE_MODERN_API=ON \
    -DENABLE_MINIMAL_BUILD=ON \
    ..
```

## Platform-Specific Instructions

### Raspberry Pi

#### Pi 4 Optimization
```bash
cmake \
    -DRPI_BUILD=ON \
    -DRPI_VERSION=4 \
    -DENABLE_OMX=ON \
    -DENABLE_MODERN_API=ON \
    -DCMAKE_CXX_FLAGS="-mcpu=cortex-a72 -mfpu=neon-fp-armv8" \
    ..
```

#### Pi Zero/3 Optimization
```bash
cmake \
    -DRPI_BUILD=ON \
    -DRPI_VERSION=3 \
    -DENABLE_MODERN_API=ON \
    -DCMAKE_CXX_FLAGS="-mcpu=cortex-a53 -mfpu=neon-fp-armv8" \
    ..
```

### Windows (MSYS2)

#### Setup MSYS2 Environment
```bash
# Install MSYS2 and update
pacman -Syu

# Install dependencies
pacman -S \
    mingw-w64-x86_64-gcc \
    mingw-w64-x86_64-cmake \
    mingw-w64-x86_64-pkg-config \
    mingw-w64-x86_64-boost \
    mingw-w64-x86_64-protobuf \
    mingw-w64-x86_64-qt5
```

#### Build on Windows
```bash
cmake \
    -G "MSYS Makefiles" \
    -DENABLE_MODERN_API=ON \
    -DWIN32_BUILD=ON \
    ..

make -j$(nproc)
```

## Troubleshooting Build Issues

### Common Issues

#### 1. Missing Dependencies
```bash
# Check for missing packages
ldd ./autoapp

# Install missing Qt plugins
sudo apt install qtbase5-dev-tools

# Update package database
sudo apt update && sudo apt upgrade
```

#### 2. CMake Configuration Errors
```bash
# Clear CMake cache
rm -rf CMakeCache.txt CMakeFiles/

# Verbose CMake output
cmake --debug-output ..

# Check CMAKE variables
cmake -LAH ..
```

#### 3. Compilation Errors
```bash
# Increase compiler verbosity
make VERBOSE=1

# Check compiler version
g++ --version
cmake --version

# Use different compiler
export CC=clang
export CXX=clang++
```

#### 4. Linking Errors
```bash
# Check library paths
ldconfig -p | grep boost

# Add library paths
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# Use static linking
cmake -DBUILD_SHARED_LIBS=OFF ..
```

#### 5. Modern Architecture Issues
```bash
# Ensure modern API is enabled
cmake -DENABLE_MODERN_API=ON ..

# Check for legacy conflicts
find . -name "*EventBus*" -not -path "./include/modern/*" -not -path "./src/modern/*"

# Clean build completely
rm -rf build/ && mkdir build && cd build
```

### Debug Build Configuration

```bash
cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="-g -O0 -Wall -Wextra" \
    -DENABLE_SANITIZERS=ON \
    -DENABLE_MODERN_API=ON \
    ..
```

### Memory Debugging

```bash
# Build with AddressSanitizer
cmake \
    -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer" \
    -DENABLE_MODERN_API=ON \
    ..

# Run with Valgrind
valgrind --leak-check=full ./autoapp
```

## Build Verification

### Automated Tests
```bash
# Run all tests
ctest --verbose

# Run specific test suite
ctest -R "logger_tests"

# Run tests with memory checking
ctest -T memcheck
```

### Manual Verification
```bash
# Check binary dependencies
ldd autoapp
objdump -p autoapp | grep NEEDED

# Verify modern components
strings autoapp | grep -i "modern\|logger\|event"

# Check symbol table
nm autoapp | grep Logger
```

## Continuous Integration

### GitHub Actions Example
```yaml
name: Build OpenAuto

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v3
      with:
        submodules: recursive
        
    - name: Install dependencies
      run: |
        sudo apt update
        sudo apt install -y build-essential cmake libboost-all-dev qtbase5-dev
        
    - name: Configure
      run: |
        mkdir build && cd build
        cmake -DENABLE_MODERN_API=ON ..
        
    - name: Build
      run: |
        cd build
        make -j$(nproc)
        
    - name: Test
      run: |
        cd build
        ctest --verbose
```

This build guide covers all aspects of building OpenAuto with the modern architecture, from basic setup to advanced optimization and troubleshooting.
