# OpenAuto Build Guide

## Overview

This guide provides comprehensive instructions for building OpenAuto with the modern architecture, including the new logger, event bus, state machine, and REST API components.

For complete details about OpenAuto's date-based versioning system and VS Code development environment, see the **[Version Scheme and VS Code Integration Guide](version-scheme-and-vscode.md)**.

## Quick Start with VS Code

If you're using VS Code, OpenAuto includes **18 specialized tasks** for complete development automation:

### Essential VS Code Tasks

| Task | Purpose | Shortcut |
|------|---------|----------|
| **Build Release Package** | Build production package | `Ctrl+Shift+P` → "Tasks: Run Build Task" |
| **Run Tests** | Execute test suite | `Ctrl+Shift+P` → "Tasks: Run Test Task" |
| **Full Build and Test Pipeline** | Complete CI/CD workflow | `Ctrl+Shift+P` → "Tasks: Run Task" |

### VS Code Setup

1. **Open Workspace**: Open `/home/pi/openauto` in VS Code
2. **Install Extensions**: C/C++, CMake Tools (recommended)  
3. **Run Tasks**: `Ctrl+Shift+P` → "Tasks: Run Task" → Select any task
4. **View Problems**: Compilation errors appear in Problems panel automatically

See the [Version Scheme and VS Code Integration Guide](version-scheme-and-vscode.md) for complete task documentation.

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

#### Development Build with Debug Logging
```bash
cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_MODERN_API=ON \
    -DENABLE_REST_API=ON \
    -DENABLE_EVENT_BUS=ON \
    -DENABLE_STATE_MACHINE=ON \
    -DENABLE_SANITIZERS=ON \
    -DNOPI=ON \
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

## Debug Logging

### Enable Debug Mode

To enable comprehensive debug logging including AASDK debug information:

#### Method 1: Build Configuration + Environment Variables
```bash
# 1. Build with debug configuration
cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_MODERN_API=ON \
    -DENABLE_REST_API=ON \
    -DENABLE_EVENT_BUS=ON \
    -DENABLE_STATE_MACHINE=ON \
    -DENABLE_SANITIZERS=ON \
    -DNOPI=ON \
    ..

make -j$(nproc)

# 2. Configure debug logging
./configure_debug_logging.sh

# 3. Run with debug logging
./autoapp
```

#### Method 2: Manual Environment Setup
```bash
# Set environment variables
export OPENAUTO_LOG_LEVEL=DEBUG
export AASDK_LOG_LEVEL=DEBUG
export OPENAUTO_DEBUG_MODE=1

# Run autoapp
./autoapp
```

#### Method 3: Quick Debug Session
```bash
# One-liner for debug session
OPENAUTO_DEBUG_MODE=1 OPENAUTO_LOG_LEVEL=DEBUG ./autoapp
```

### Log Levels and Categories

**Available Log Levels:**
- `TRACE` - Most verbose, includes function entry/exit
- `DEBUG` - Detailed operational information
- `INFO` - General informational messages  
- `WARN` - Warning messages
- `ERROR` - Error conditions
- `FATAL` - Critical errors

**Log Categories:**
- `ANDROID_AUTO` - Android Auto protocol and communication
- `SYSTEM` - System-level operations
- `UI` - User interface events and updates
- `CAMERA` - Camera operations and video processing
- `NETWORK` - Network communications
- `BLUETOOTH` - Bluetooth operations
- `AUDIO` - Audio processing and playback
- `VIDEO` - Video processing and display
- `CONFIG` - Configuration management
- `API` - REST API operations
- `EVENT` - Event bus messages
- `STATE` - State machine transitions

### Debug Output Locations

When debug logging is enabled, you'll see output in:

1. **Console (stdout)** - Real-time colored debug output
2. **Log file** - `/tmp/openauto-debug.log` (if configured)
3. **Application log** - `autoapp.log` in current directory

### Sample Debug Output

```
[2025-07-18 10:30:15.123] [DEBUG] [ANDROID_AUTO] [autoapp] 🔍 Starting Android Auto handshake
[2025-07-18 10:30:15.124] [DEBUG] [SYSTEM] [autoapp] USB device enumeration started
[2025-07-18 10:30:15.125] [DEBUG] [UI] [autoapp] MainWindow initialized with resolution 1920x1080
[2025-07-18 10:30:15.126] [DEBUG] [CAMERA] [autoapp] Camera service initialized
```

### Troubleshooting Debug Logging

**Issue: No debug output appearing**
```bash
# Check environment variables
echo $OPENAUTO_LOG_LEVEL
echo $OPENAUTO_DEBUG_MODE

# Verify build configuration
strings ./autoapp | grep -i debug
```

**Issue: Too much log output**
```bash
# Filter specific categories
OPENAUTO_LOG_LEVEL=INFO ./autoapp 2>&1 | grep "ANDROID_AUTO"

# Save to file for analysis
OPENAUTO_DEBUG_MODE=1 ./autoapp > debug_output.log 2>&1
```

**Issue: Missing AASDK debug info**
- Ensure you're using `-DCMAKE_BUILD_TYPE=Debug`
- AASDK debug is compiled in during debug builds
- Check that AASDK library was built with debug symbols

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

### Version Information Check
```bash
# Check version information during build
./check-version.sh

# Expected output:
# 🔍 OpenAuto Version Information
# ==================================
# 📅 Date-based Version Components:
#   Major (Year):  2025
#   Minor (Month): 07
#   Patch (Day):   20
# 🔗 Git Information:
#   Branch:        crankshaft-ng_2025
#   Commit:        fc4e9d0
#   Working Tree:  CLEAN
# 🎯 Final Version: 2025.07.20+fc4e9d0

# Verify version embedded in binary
strings autoapp | grep "2025\."
```

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
