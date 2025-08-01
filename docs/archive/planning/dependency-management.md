# Dependency Management Guide

This document provides comprehensive guidance on managing dependencies for OpenAuto across different platforms and build environments.

## Table of Contents
- [Overview](#overview)
- [Core Dependencies](#core-dependencies)
- [Platform-Specific Dependencies](#platform-specific-dependencies)
- [Dependency Management Tools](#dependency-management-tools)
- [Version Constraints](#version-constraints)
- [Installation Methods](#installation-methods)
- [Troubleshooting](#troubleshooting)

## Overview

OpenAuto requires various external libraries and tools for compilation and runtime. This guide covers:
- Required vs optional dependencies
- Version compatibility matrix
- Installation methods per platform
- Dependency resolution strategies
- Common troubleshooting scenarios

### Dependency Categories

1. **Build Dependencies**: Required for compilation
2. **Runtime Dependencies**: Required for execution
3. **Development Dependencies**: Required for development/testing
4. **Optional Dependencies**: Provide additional features

## Core Dependencies

### Required Build Dependencies

#### CMake (Build System)
- **Minimum Version**: 3.16
- **Recommended Version**: 3.20+
- **Purpose**: Build configuration and generation
- **Installation**: Package manager or cmake.org

```bash
# Ubuntu/Debian
sudo apt install cmake

# CentOS/RHEL
sudo yum install cmake3

# macOS
brew install cmake

# Windows (via Chocolatey)
choco install cmake
```

#### C++ Compiler
- **Standard**: C++17 or later
- **Supported Compilers**:
  - GCC 9.0+
  - Clang 10.0+
  - MSVC 2019+

```bash
# Ubuntu/Debian
sudo apt install build-essential

# CentOS/RHEL
sudo yum groupinstall "Development Tools"

# macOS
xcode-select --install
```

#### Boost Libraries
- **Minimum Version**: 1.70
- **Recommended Version**: 1.75+
- **Required Components**:
  - boost_system
  - boost_filesystem
  - boost_thread
  - boost_log
  - boost_program_options

```bash
# Ubuntu/Debian
sudo apt install libboost-all-dev

# CentOS/RHEL
sudo yum install boost-devel

# macOS
brew install boost

# Windows (vcpkg)
vcpkg install boost:x64-windows
```

#### Protocol Buffers
- **Minimum Version**: 3.10
- **Purpose**: Message serialization for Android Auto protocol
- **Components**: libprotobuf, protoc compiler

```bash
# Ubuntu/Debian
sudo apt install libprotobuf-dev protobuf-compiler

# CentOS/RHEL
sudo yum install protobuf-devel

# macOS
brew install protobuf

# Windows (vcpkg)
vcpkg install protobuf:x64-windows
```

#### Qt5 Framework
- **Minimum Version**: 5.12
- **Required Modules**:
  - Qt5Core
  - Qt5Widgets
  - Qt5Multimedia
  - Qt5Quick (Declarative)
  - Qt5QuickControls2
  - Qt5Network

```bash
# Ubuntu/Debian
sudo apt install qtbase5-dev qtmultimedia5-dev qtdeclarative5-dev qtquickcontrols2-5-dev

# CentOS/RHEL
sudo yum install qt5-qtbase-devel qt5-qtmultimedia-devel qt5-qtdeclarative-devel

# macOS
brew install qt5

# Windows (vcpkg)
vcpkg install qt5:x64-windows
```

#### JSON Library
- **Library**: nlohmann/json
- **Minimum Version**: 3.7.0
- **Purpose**: Configuration and API data handling

```bash
# Ubuntu/Debian
sudo apt install nlohmann-json3-dev

# CentOS/RHEL (EPEL required)
sudo yum install json-devel

# macOS
brew install nlohmann-json

# Windows (vcpkg)
vcpkg install nlohmann-json:x64-windows
```

### Android Auto Specific Dependencies

#### AASDK (Android Auto SDK)
- **Repository**: https://github.com/opencardev/aasdk
- **Purpose**: Android Auto protocol implementation
- **Build**: Must be built from source

```bash
# Clone and build AASDK
git clone https://github.com/opencardev/aasdk.git
cd aasdk
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DTARGET_ARCH=amd64
make -j$(nproc)
sudo make install
```

#### LibUSB
- **Version**: 1.0+
- **Purpose**: USB communication with Android devices

```bash
# Ubuntu/Debian
sudo apt install libusb-1.0-0-dev

# CentOS/RHEL
sudo yum install libusb1-devel

# macOS
brew install libusb

# Windows (vcpkg)
vcpkg install libusb:x64-windows
```

### Audio Dependencies

#### RtAudio
- **Purpose**: Real-time audio I/O
- **Version**: 5.1.0+

```bash
# Ubuntu/Debian
sudo apt install librtaudio-dev

# Build from source if not available
git clone https://github.com/thestk/rtaudio.git
cd rtaudio
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
sudo make install
```

#### TagLib
- **Purpose**: Audio metadata reading
- **Version**: 1.11+

```bash
# Ubuntu/Debian
sudo apt install libtag1-dev

# CentOS/RHEL
sudo yum install taglib-devel

# macOS
brew install taglib

# Windows (vcpkg)
vcpkg install taglib:x64-windows
```

### Development Dependencies

#### Google Test
- **Purpose**: Unit and integration testing
- **Version**: 1.10+

```bash
# Ubuntu/Debian
sudo apt install libgtest-dev

# Build if only source is available
cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make
sudo cp lib/*.a /usr/lib

# macOS
brew install googletest

# Windows (vcpkg)
vcpkg install gtest:x64-windows
```

#### Google Mock
- **Purpose**: Mocking framework for testing
- **Usually included with Google Test**

## Platform-Specific Dependencies

### Linux Specific

#### D-Bus (Optional)
- **Purpose**: Inter-process communication
- **Installation**:
```bash
sudo apt install libdbus-1-dev  # Ubuntu/Debian
sudo yum install dbus-devel     # CentOS/RHEL
```

#### PulseAudio (Optional)
- **Purpose**: Audio system integration
- **Installation**:
```bash
sudo apt install libpulse-dev  # Ubuntu/Debian
sudo yum install pulseaudio-libs-devel  # CentOS/RHEL
```

### Windows Specific

#### Windows SDK
- **Purpose**: Windows-specific APIs
- **Installation**: Visual Studio Installer

#### vcpkg Integration
```powershell
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install

# Install all dependencies
.\vcpkg install --triplet x64-windows ^
    boost ^
    protobuf ^
    nlohmann-json ^
    qt5 ^
    gtest ^
    libusb ^
    taglib
```

### Raspberry Pi Specific

#### Hardware-Specific Libraries
```bash
# Raspberry Pi GPIO libraries
sudo apt install wiringpi

# Camera support
sudo apt install libraspberrypi-dev

# Hardware acceleration
sudo apt install libomxil-bellagio-dev
```

## Dependency Management Tools

### Package Managers

#### vcpkg (Windows, Cross-platform)
```cmake
# CMakeLists.txt integration
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
  CACHE STRING "Vcpkg toolchain file")
```

#### Conan (Cross-platform)
```ini
# conanfile.txt
[requires]
boost/1.75.0
protobuf/3.17.1
nlohmann_json/3.9.1
qt/5.15.2
gtest/1.11.0

[generators]
cmake
```

#### Hunter (CMake-based)
```cmake
# CMakeLists.txt
include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/cpp-pm/hunter/archive/v0.23.297.tar.gz"
    SHA1 "3319fe6a3b08090df7df98dee75134d68e2ef5a3"
)
```

### Dependency Version Matrix

| Component | Ubuntu 18.04 | Ubuntu 20.04 | Ubuntu 22.04 | CentOS 8 | Windows | macOS |
| --------- | ------------ | ------------ | ------------ | -------- | ------- | ----- |
| CMake     | 3.10.2       | 3.16.3       | 3.22.1       | 3.20.2   | 3.24+   | 3.24+ |
| GCC       | 7.5.0        | 9.4.0        | 11.2.0       | 8.5.0    | -       | -     |
| Boost     | 1.65.1       | 1.71.0       | 1.74.0       | 1.66.0   | 1.79+   | 1.79+ |
| Qt5       | 5.9.5        | 5.12.8       | 5.15.3       | 5.15.2   | 5.15+   | 5.15+ |
| Protobuf  | 3.0.0        | 3.6.1        | 3.12.4       | 3.5.0    | 3.17+   | 3.17+ |

### Version Constraint Handling

```cmake
# CMakeLists.txt version requirements
cmake_minimum_required(VERSION 3.16)

# Find packages with version constraints
find_package(Boost 1.70 REQUIRED COMPONENTS system filesystem thread log program_options)
find_package(Protobuf 3.10 REQUIRED)
find_package(Qt5 5.12 REQUIRED COMPONENTS Core Widgets Multimedia Quick Network)
find_package(nlohmann_json 3.7.0 REQUIRED)
find_package(GTest 1.10 REQUIRED)

# Check compiler version
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
        message(FATAL_ERROR "GCC version must be at least 9.0")
    endif()
endif()
```

## Installation Methods

### System Package Manager
**Pros**: Easy installation, system integration
**Cons**: Version may be outdated, system-wide changes

### Source Compilation
**Pros**: Latest version, custom configuration
**Cons**: Time-consuming, requires build tools

### Package Manager (vcpkg, Conan)
**Pros**: Version control, reproducible builds
**Cons**: Additional setup, learning curve

### Containerized Dependencies
**Pros**: Isolated environment, reproducible
**Cons**: Additional overhead, Docker knowledge required

```dockerfile
# Dockerfile for dependency management
FROM ubuntu:20.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libboost-all-dev \
    libprotobuf-dev \
    protobuf-compiler \
    nlohmann-json3-dev \
    qtbase5-dev \
    qtmultimedia5-dev \
    libgtest-dev \
    libusb-1.0-0-dev \
    libtag1-dev \
    && rm -rf /var/lib/apt/lists/*
```

## Troubleshooting

### Common Issues

#### Missing Package Config Files
```bash
# Install pkg-config
sudo apt install pkg-config

# Check package availability
pkg-config --list-all | grep -i boost
```

#### Version Conflicts
```cmake
# CMakeLists.txt - Force specific version
set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost 1.70 EXACT REQUIRED COMPONENTS system filesystem)
```

#### Library Not Found
```bash
# Update library cache
sudo ldconfig

# Check library paths
echo $LD_LIBRARY_PATH

# Set library path
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

#### CMake Module Path Issues
```cmake
# Add custom module paths
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")
```

### Dependency Verification

#### Check Installed Versions
```bash
# CMake
cmake --version

# GCC
gcc --version

# Qt5
qmake --version

# Boost
cat /usr/include/boost/version.hpp | grep "BOOST_LIB_VERSION"

# Protobuf
protoc --version
```

#### Runtime Dependency Check
```bash
# Check binary dependencies
ldd bin/autoapp  # Linux
otool -L bin/autoapp  # macOS
dumpbin /dependents autoapp.exe  # Windows
```

### Platform-Specific Issues

#### Ubuntu 18.04 CMake Issues
```bash
# Install newer CMake
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | \
  gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main'
sudo apt update
sudo apt install cmake
```

#### Windows vcpkg Issues
```powershell
# Reset vcpkg
.\vcpkg integrate remove
.\vcpkg integrate install

# Check integration
.\vcpkg integrate project
```

#### macOS Homebrew Issues
```bash
# Update Homebrew
brew update
brew upgrade

# Fix permissions
sudo chown -R $(whoami) $(brew --prefix)/*
```

## Best Practices

### Dependency Pinning
```cmake
# Use FetchContent for specific versions
include(FetchContent)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.9.1
)

FetchContent_MakeAvailable(nlohmann_json)
```

### Dependency Documentation
- Document all dependencies in README.md
- Maintain compatibility matrix
- Document installation procedures
- Include troubleshooting notes

### Automated Dependency Management
```yaml
# .github/workflows/dependencies.yml
name: Update Dependencies
on:
  schedule:
    - cron: '0 0 * * 0'  # Weekly
  workflow_dispatch:

jobs:
  update:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Update vcpkg
        run: |
          cd vcpkg
          git pull
          ./bootstrap-vcpkg.sh
```

## Related Documentation

- [Build Guide](build-guide.md) - Complete build instructions
- [Cross-Platform Builds](cross-platform-builds.md) - Platform-specific builds
- [Container Deployment](container-deployment.md) - Containerized dependency management
- [Troubleshooting Guide](troubleshooting-guide.md) - General troubleshooting

## Contributing

When adding new dependencies:
1. Update this document
2. Update build scripts
3. Test on all supported platforms
4. Update CI/CD pipelines
5. Document any special requirements
