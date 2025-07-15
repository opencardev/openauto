# Cross-Platform Build Guide

This document provides platform-specific build instructions for OpenAuto, covering Windows, Linux, macOS, and embedded systems.

## Table of Contents
- [Build Overvie# Install dependencies
sudo apt install -y \
    build-essential \
    cmake \
    pkg-config \
    libboost-all-dev \
    libprotobuf-dev \
    protobuf-compiler \
    nlohmann-json3-dev \
    qtbase5-dev \
    qtmultimedia5-dev \
    qtdeclarative5-dev \
    qtquickcontrols2-5-dev \
    libgtest-dev \
    libasio-dev \
    libusb-1.0-0-dev \
    libtag1-dev \
    librtaudio-devew)
- [Windows Build](#windows-build)
- [Linux Build](#linux-build)
- [macOS Build](#macos-build)
- [Raspberry Pi Build](#raspberry-pi-build)
- [Cross-Compilation](#cross-compilation)
- [Architecture-Specific Notes](#architecture-specific-notes)

## Build Overview

OpenAuto supports multiple build configurations:
- **Native builds**: Direct compilation on target platform
- **Cross-compilation**: Build for different architecture
- **Container builds**: Reproducible builds using Docker
- **CI/CD builds**: Automated pipeline builds

### Build Matrix

| Platform | Architecture | Compiler | Status |
|----------|-------------|----------|--------|
| Windows | x64 | MSVC 2019+ | ✅ Supported |
| Windows | x64 | MinGW-w64 | ✅ Supported |
| Linux | x64 | GCC 9+ | ✅ Supported |
| Linux | ARM64 | GCC 9+ | ✅ Supported |
| Linux | ARMv7 | GCC 9+ | ✅ Supported |
| macOS | x64 | Clang 12+ | ⚠️ Limited |
| macOS | ARM64 | Clang 12+ | ⚠️ Limited |

## Windows Build

### Prerequisites
```powershell
# Install Visual Studio 2019 or later with C++ development tools
# Install CMake 3.16+
# Install vcpkg for dependency management

# Clone vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
```

### Dependencies via vcpkg
```powershell
# Install required dependencies
.\vcpkg install boost:x64-windows
.\vcpkg install protobuf:x64-windows
.\vcpkg install nlohmann-json:x64-windows
.\vcpkg install qt5:x64-windows
.\vcpkg install gtest:x64-windows
```

### Build Commands
```powershell
# Configure with vcpkg toolchain
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows

# Build
cmake --build build --config Release

# Run tests
cd build
ctest -C Release
```

### Windows-Specific Configuration
```cmake
# CMakeLists.txt additions for Windows
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
endif()
```

## Linux Build

### Ubuntu/Debian Prerequisites
```bash
# Update package list
sudo apt update

# Install build tools
sudo apt install -y build-essential cmake pkg-config

# Install dependencies
sudo apt install -y \
    libboost-all-dev \
    libprotobuf-dev \
    protobuf-compiler \
    nlohmann-json3-dev \
    qtbase5-dev \
    qtmultimedia5-dev \
    libgtest-dev \
    libasio-dev \
    libusb-1.0-0-dev \
    libtag1-dev \
    librtaudio-dev
```

### CentOS/RHEL Prerequisites
```bash
# Install EPEL repository
sudo yum install -y epel-release

# Install build tools
sudo yum groupinstall -y "Development Tools"
sudo yum install -y cmake3 pkgconfig

# Install dependencies
sudo yum install -y \
    boost-devel \
    protobuf-devel \
    json-devel \
    qt5-qtbase-devel \
    qt5-qtmultimedia-devel \
    qt5-qtdeclarative-devel \
    gtest-devel \
    libusb1-devel \
    taglib-devel
```

### Build Commands
```bash
# Configure
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Build with parallel jobs
cmake --build build -j$(nproc)

# Install
sudo cmake --install build

# Run tests
cd build
ctest --parallel $(nproc)
```

### Linux Distribution-Specific Notes

#### Ubuntu 18.04 LTS
```bash
# Requires newer CMake
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main'
sudo apt update
sudo apt install cmake
```

#### Arch Linux
```bash
# Install dependencies
sudo pacman -S base-devel cmake boost protobuf nlohmann-json qt5-base qt5-multimedia gtest libusb taglib rtaudio
```

## macOS Build

### Prerequisites
```bash
# Install Homebrew
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install Xcode command line tools
xcode-select --install

# Install dependencies
brew install cmake boost protobuf nlohmann-json qt5 googletest libusb taglib rtaudio
```

### Build Commands
```bash
# Configure with Homebrew paths
cmake -B build -S . \
    -DCMAKE_BUILD_TYPE=Release \
    -DQt5_DIR=$(brew --prefix qt5)/lib/cmake/Qt5

# Build
cmake --build build -j$(sysctl -n hw.ncpu)

# Run tests
cd build
ctest --parallel $(sysctl -n hw.ncpu)
```

### macOS-Specific Issues
- Qt5 may require additional path configuration
- Some Android Auto libraries may not be available
- Limited testing on Apple Silicon (M1/M2)

## Raspberry Pi Build

### Raspberry Pi OS Setup
```bash
# Update system
sudo apt update && sudo apt upgrade -y

# Install dependencies (same as Debian/Ubuntu)
sudo apt install -y \
    build-essential \
    cmake \
    pkg-config \
    libboost-all-dev \
    libprotobuf-dev \
    protobuf-compiler \
    nlohmann-json3-dev \
    qtbase5-dev \
    qtmultimedia5-dev \
    qtdeclarative5-dev \
    qtquickcontrols2-5-dev \
    libgtest-dev \
    libasio-dev \
    libusb-1.0-0-dev \
    libtag1-dev \
    librtaudio-dev
```

### Build Configuration
```bash
# Configure for Raspberry Pi
cmake -B build -S . \
    -DCMAKE_BUILD_TYPE=Release \
    -DRASPBERRY_PI_BUILD=ON \
    -DCMAKE_CXX_FLAGS="-mcpu=cortex-a72 -mfpu=neon-fp-armv8"

# Build (may take considerable time)
cmake --build build -j2  # Limit parallel jobs to avoid memory issues
```

### Raspberry Pi Optimization
```cmake
# CMakeLists.txt for Raspberry Pi
if(RASPBERRY_PI_BUILD)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=cortex-a72")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon-fp-armv8")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()
```

## Cross-Compilation

### Docker Cross-Compilation Environment
```dockerfile
# Dockerfile.cross-aarch64
FROM ubuntu:20.04

RUN apt-get update && apt-get install -y \
    gcc-aarch64-linux-gnu \
    g++-aarch64-linux-gnu \
    cmake \
    pkg-config

# Install cross-compiled dependencies
# ... (dependency installation for target architecture)

ENV CC=aarch64-linux-gnu-gcc
ENV CXX=aarch64-linux-gnu-g++
```

### Cross-Compilation Toolchain
```cmake
# toolchain-aarch64.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
```

### Cross-Compilation Commands
```bash
# Configure for cross-compilation
cmake -B build-aarch64 -S . \
    -DCMAKE_TOOLCHAIN_FILE=toolchain-aarch64.cmake \
    -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build-aarch64
```

## Architecture-Specific Notes

### x86_64 (Intel/AMD)
- Full feature support
- Optimal performance
- All dependencies available

### ARM64 (AArch64)
- Full feature support
- Good performance
- Some dependencies may require compilation from source

### ARMv7 (32-bit ARM)
- Limited memory considerations
- Some features may be disabled for performance
- Requires careful dependency management

### Build Performance Tips

#### Parallel Builds
```bash
# Use all available cores
cmake --build build -j$(nproc)

# Limit memory usage on constrained systems
cmake --build build -j2
```

#### Compilation Database
```bash
# Generate compile_commands.json for IDEs
cmake -B build -S . -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

#### Ccache Integration
```bash
# Install ccache for faster rebuilds
sudo apt install ccache

# Configure CMake to use ccache
cmake -B build -S . -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
```

## Build Troubleshooting

### Common Issues

#### Missing Dependencies
```bash
# Check CMake dependency resolution
cmake -B build -S . --debug-output
```

#### Memory Issues During Build
```bash
# Reduce parallel jobs
cmake --build build -j1

# Or increase swap space
sudo fallocate -l 2G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile
```

#### Qt5 Detection Issues
```bash
# Explicitly set Qt5 path
cmake -B build -S . -DQt5_DIR=/usr/lib/x86_64-linux-gnu/cmake/Qt5
```

### Build Verification
```bash
# Run full test suite
cd build
ctest --output-on-failure

# Check binary dependencies
ldd bin/autoapp  # Linux
otool -L bin/autoapp  # macOS
```

## Continuous Integration

For automated builds, see:
- [GitHub Actions Pipeline](.github/workflows/ci.yml)
- [Container Build Guide](container-deployment.md)
- [Test Automation](test-automation.md)

## Next Steps

After successful build:
1. [Deployment Guide](deployment-guide.md) - Deploy the application
2. [Test Guide](test-guide.md) - Run comprehensive tests
3. [Troubleshooting Guide](troubleshooting-guide.md) - Handle issues
