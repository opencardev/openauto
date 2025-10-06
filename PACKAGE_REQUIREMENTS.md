# OpenAuto Package Requirements

## Overview

This document outlines the updated package requirements for OpenAuto, including the recent RtAudio 6+ compatibility changes that support both legacy and modern RtAudio versions.

**Last Updated**: October 5, 2025  
**OpenAuto Version**: 2025.10.05+git.current  
**RtAudio Support**: 5.x and 6.x+ (dual compatibility)

## Quick Install Commands

### Ubuntu/Debian (Recommended)
```bash
# Core build dependencies
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
    libudev-dev \
    qtbase5-dev \
    qtmultimedia5-dev \
    qtconnectivity5-dev \
    libqt5multimedia5-plugins \
    libqt5bluetooth5 \
    libqt5bluetooth5-bin \
    libasound2-dev \
    libpulse-dev \
    librtaudio-dev \
    libtag1-dev \
    nlohmann-json3-dev \
    libgps-dev \
    gpsd \
    gpsd-clients \
    libcpp-httplib-dev

# Runtime dependencies (for package installation)
sudo apt install -y \
    libprotobuf32t64 \
    libusb-1.0-0 \
    libtag2 \
    librtaudio7 \
    libasound2t64 \
    libpulse0 \
    libqt5multimedia5 \
    libqt5bluetooth5 \
    libgps30t64 \
    gpsd \
    libcpp-httplib0.18
```

### Arch Linux
```bash
# Development packages
sudo pacman -S \
    base-devel \
    cmake \
    pkgconf \
    git \
    boost \
    protobuf \
    openssl \
    libusb \
    systemd \
    qt5-base \
    qt5-multimedia \
    qt5-connectivity \
    alsa-lib \
    libpulse \
    rtaudio \
    taglib \
    nlohmann-json \
    gpsd \
    cpp-httplib
```

### Fedora/RHEL/CentOS
```bash
# Development packages
sudo dnf install -y \
    gcc-c++ \
    cmake \
    pkgconfig \
    git \
    boost-devel \
    protobuf-devel \
    openssl-devel \
    libusb1-devel \
    systemd-devel \
    qt5-qtbase-devel \
    qt5-qtmultimedia-devel \
    qt5-qtconnectivity-devel \
    alsa-lib-devel \
    pulseaudio-libs-devel \
    rtaudio-devel \
    taglib-devel \
    json-devel \
    gpsd-devel \
    gpsd \
    cpp-httplib-devel
```

## Detailed Package Information

### Core Dependencies

#### Build System
- **cmake** (>= 3.5.1) - Build system generator
- **build-essential** / **base-devel** - C++ compiler toolchain
- **pkg-config** / **pkgconf** - Package configuration system
- **git** - Version control (for submodules)

#### C++ Libraries
- **libboost-all-dev** / **boost** - Boost C++ libraries (system, log_setup, log, unit_test_framework)
- **libprotobuf-dev** / **protobuf** - Protocol Buffers library
- **protobuf-compiler** - Protocol Buffers compiler
- **libssl-dev** / **openssl** - SSL/TLS library
- **libusb-1.0-0-dev** / **libusb** - USB device access library
- **libudev-dev** / **systemd** - Device management library

#### Qt Framework
- **qtbase5-dev** / **qt5-base** - Qt5 core development files
- **qtmultimedia5-dev** / **qt5-multimedia** - Qt5 multimedia framework
- **qtconnectivity5-dev** / **qt5-connectivity** - Qt5 connectivity (Bluetooth)
- **libqt5multimedia5-plugins** - Qt5 multimedia plugins
- **libqt5bluetooth5** - Qt5 Bluetooth runtime
- **libqt5bluetooth5-bin** - Qt5 Bluetooth utilities

#### Audio System
- **libasound2-dev** / **alsa-lib** - ALSA audio library
- **libpulse-dev** / **libpulse** - PulseAudio library
- **librtaudio-dev** / **rtaudio** - **RtAudio library (5.x or 6.x+ supported)**
- **libtag1-dev** / **taglib** - Audio metadata library

#### Modern Architecture
- **nlohmann-json3-dev** / **nlohmann-json** - JSON library for REST API
- **libcpp-httplib-dev** - C++ HTTP/HTTPS server and client library (required for REST API)
- **libevent-dev** - Event notification library (optional)

#### GPS Support (Required)
- **libgps-dev** / **libgps** - GPS development libraries
- **gpsd** - GPS daemon for location services
- **gpsd-clients** - GPS client tools and utilities

### Runtime Dependencies

When installing the built package, these runtime libraries are required:

```bash
# Ubuntu/Debian runtime dependencies
libprotobuf32t64
libusb-1.0-0
libtag2
librtaudio7        # RtAudio 7.x runtime (backward compatible with 6.x)
libasound2t64
libpulse0
libqt5multimedia5
libqt5bluetooth5
libqt5core5a
libqt5gui5
libqt5widgets5
libqt5network5
libqt5dbus5
libgps30t64        # GPS runtime library
gpsd               # GPS daemon
libcpp-httplib0.18 # HTTP library for REST API
```

### Optional Dependencies

#### Enhanced Bluetooth Support
```bash
# Ubuntu/Debian
sudo apt install -y libbluetooth-dev bluez bluez-tools

# Arch Linux
sudo pacman -S bluez bluez-utils
```

#### Video Acceleration (Raspberry Pi)
```bash
# Raspberry Pi specific
sudo apt install -y \
    libomxil-bellagio-dev \
    libraspberrypi-dev \
    libavformat-dev \
    libavcodec-dev \
    libavutil-dev \
    libswscale-dev
```

## RtAudio Version Compatibility

### Recent Update: RtAudio 6+ Support

OpenAuto now supports both legacy and modern RtAudio versions:

- **RtAudio 5.x and earlier**: Uses exception-based error handling
- **RtAudio 6.x and later**: Uses return-code-based error handling

### Include Path Support

The code automatically detects and uses the correct include path:
- `<rtaudio/RtAudio.h>` (modern installations)
- `<RtAudio.h>` (legacy installations)

### Version Detection

The build system automatically detects RtAudio version at compile time:
```cpp
#if defined(RTAUDIO_VERSION_MAJOR) && (RTAUDIO_VERSION_MAJOR >= 6)
#  define OA_RTAUDIO_V6 1
#endif
```

### Installation Options

#### Option 1: System Package (Recommended)
```bash
# Ubuntu/Debian - may install RtAudio 5.x or 6.x depending on distribution
sudo apt install librtaudio-dev

# Arch Linux - typically provides latest RtAudio 6.x
sudo pacman -S rtaudio
```

#### Option 2: Build from Source (Latest)
```bash
# Build latest RtAudio 6.x from source
git clone https://github.com/thestk/rtaudio.git
cd rtaudio
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo make install
```

## Platform-Specific Notes

### Package Naming Updates (October 2025)

Recent Ubuntu/Debian distributions have updated package names:
- `libprotobuf32` → `libprotobuf32t64`
- `libasound2` → `libasound2t64`  
- `libtag1v5` → `libtag2`
- `librtaudio6` → `librtaudio7`
- `libhttplib-dev` → `libcpp-httplib-dev`

These changes reflect updated library versions and ABI transitions in modern distributions.

### Library API Updates (October 2025)

Recent library updates have changed some APIs:
- **TagLib 2.x**: `TagLib::uint` → `unsigned int` (affects track number handling)
- **RtAudio 6.x+**: Exception handling → Return code handling (auto-detected)

### GPS Requirement Update (October 2025)

GPS support is now a **required dependency** for OpenAuto builds. The GPS libraries (`libgps-dev`, `gpsd`) are no longer optional and must be installed for successful compilation.

If you don't need GPS functionality, you may need to modify the CMakeLists.txt to make GPS optional or use an older version of OpenAuto.

### Ubuntu 20.04 LTS
- Uses RtAudio 5.x by default
- Fully supported with legacy compatibility mode

### Ubuntu 22.04 LTS / 24.04 LTS
- May include RtAudio 6.x
- Automatic version detection handles both cases

### Debian 11 (Bullseye)
- Uses RtAudio 5.x
- Fully supported with legacy compatibility mode

### Debian 12 (Bookworm)
- Includes RtAudio 6.x
- Uses modern error handling automatically

### Arch Linux
- Rolling release with latest RtAudio 6.x
- Uses modern error handling automatically

### Raspberry Pi OS
- Based on Debian, typically uses RtAudio 5.x
- Legacy compatibility mode ensures full functionality

## Verification Commands

### Check Installed Packages
```bash
# Ubuntu/Debian
dpkg -l | grep -E "(rtaudio|boost|protobuf|qt5)"

# Arch Linux
pacman -Qs rtaudio boost protobuf qt5

# Check RtAudio version specifically
pkg-config --modversion rtaudio 2>/dev/null || echo "RtAudio version detection via pkg-config not available"
```

### Test RtAudio Installation
```bash
# Test if RtAudio headers are available
echo '#include <RtAudio.h>' | gcc -x c++ -E - >/dev/null 2>&1 && echo "RtAudio headers found (legacy path)"
echo '#include <rtaudio/RtAudio.h>' | gcc -x c++ -E - >/dev/null 2>&1 && echo "RtAudio headers found (modern path)"
```

## Troubleshooting

### RtAudio Issues

#### Problem: "RtAudio.h not found"
```bash
# Solution 1: Install development package
sudo apt install librtaudio-dev

# Solution 2: Check if installed in different location
find /usr -name "RtAudio.h" 2>/dev/null
```

#### Problem: "Compilation errors with RtAudio 6+"
- **Cause**: Using old OpenAuto code without 6+ support
- **Solution**: Ensure you have the latest OpenAuto code with dual compatibility

#### Problem: "Audio output not working"
```bash
# Check audio system
pulseaudio --check -v
aplay -l  # List audio devices
```

### Qt Issues

#### Problem: "Qt5 modules not found"
```bash
# Ubuntu/Debian
sudo apt install qtbase5-dev qtmultimedia5-dev qtconnectivity5-dev

# Verify Qt installation
qmake --version
```

### Build Issues

#### Problem: "Killed signal terminated program cc1plus" error
```bash
# This indicates out-of-memory during compilation
# Check available memory
free -h

# Solutions:
# 1. Reduce parallel compilation jobs
make -j1  # Use single job (slowest but uses least memory)
make -j2  # Use 2 jobs (balance between speed and memory)

# 2. Add swap space if not present
sudo fallocate -l 2G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile

# 3. Close other applications to free memory
# 4. Use the updated build script which automatically limits parallelism
```

#### Problem: "Could not find Httplib" CMake error
```bash
# Install the correct httplib package
sudo apt install -y libcpp-httplib-dev

# Verify httplib installation
find /usr -name "*httplib*" -type f 2>/dev/null | head -5

# Check if httplib headers are available
ls /usr/include/httplib* 2>/dev/null || echo "httplib headers not found in standard location"
```

#### Problem: "Could not find gps" CMake error
```bash
# This is a required dependency as of October 2025
sudo apt install -y libgps-dev gpsd gpsd-clients

# Verify GPS development files are installed
pkg-config --exists libgps && echo "GPS dev libraries found" || echo "GPS dev libraries missing"

# Check if GPS headers are available
find /usr -name "gps.h" 2>/dev/null
```

#### Problem: "Package not found" errors
```bash
# Common package naming issues (October 2025+)
# If you get "Unable to locate package" errors, try these alternatives:

# libprotobuf32 → libprotobuf32t64
# libasound2 → libasound2t64  
# libtag1v5 → libtag2
# librtaudio6 → librtaudio7
# libhttplib-dev → libcpp-httplib-dev

# Update package cache first
sudo apt update

# Search for available packages
apt search librtaudio
apt search libtag
apt search libprotobuf
```

#### Problem: "Boost libraries not found"
```bash
# Install all Boost libraries
sudo apt install libboost-all-dev

# Or install specific components
sudo apt install libboost-system-dev libboost-log-dev
```

## Package Maintenance

### Updating Dependencies

When updating package requirements:

1. **Update this document**
2. **Update build scripts** (`build-packages.sh`, `scripts/build-multiarch.sh`)
3. **Update documentation** (`docs/build-guide.md`, `README.md`)
4. **Update CI/CD** (if applicable)
5. **Test on multiple distributions**

### Version Compatibility Matrix

| Distribution | RtAudio Version | Compatibility Mode | Package Name |
|-------------|----------------|-------------------|--------------|
| Ubuntu 20.04 | 5.x | Legacy (exceptions) | librtaudio6 |
| Ubuntu 22.04+ | 6.x/7.x | Auto-detected | librtaudio7 |
| Debian 11 | 5.x | Legacy (exceptions) | librtaudio6 |
| Debian 12+ | 6.x/7.x | Modern (return codes) | librtaudio7 |
| Arch Linux | 6.x/7.x | Modern (return codes) | rtaudio |
| Raspberry Pi OS | 5.x/6.x/7.x | Auto-detected | librtaudio7 |

## Additional Resources

- **[OpenAuto Build Guide](docs/build-guide.md)** - Complete build instructions
- **[OpenAuto Documentation](docs/README.md)** - Full documentation index
- **[RtAudio Official Site](https://www.music.mcgill.ca/~gary/rtaudio/)** - RtAudio documentation
- **[Qt5 Documentation](https://doc.qt.io/qt-5/)** - Qt framework documentation

---

**Note**: This document reflects the current state as of October 5, 2025. Package versions and availability may vary between distributions. Always verify package availability with your distribution's package manager before installation.