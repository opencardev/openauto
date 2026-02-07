# OpenAuto

[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](code_of_conduct.md)

### Support project
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/donate/?hosted_button_id=YAA3LW393L46S)

For support of other platforms please contact me at f1xstudiopl@gmail.com

### Community
[![Join the chat at https://gitter.im/publiclab/publiclab](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/openauto_androidauto/Lobby)

### Description
OpenAuto is an AndroidAuto(tm) headunit emulator based on aasdk library and Qt libraries. Main goal is to run this application on the RaspberryPI 3 board computer smoothly.

[See demo video](https://www.youtube.com/watch?v=k9tKRqIkQs8)

### Supported functionalities
 - 480p, 720p and 1080p with 30 or 60 FPS
 - RaspberryPI 3 hardware acceleration support to decode video stream (up to 1080p@60!)
 - Audio playback from all audio channels (Media, System and Speech)
 - Audio input for voice commands
 - Touchscreen and buttons input
 - Bluetooth
 - Automatic launch after device hotplug
 - Automatic detection of connected Android devices
 - Wireless (WiFi) mode via head unit server (must be enabled in hidden developer settings)
 - User-friendly settings

### Supported platforms

 - Linux
 - RaspberryPI 3
 - Windows

### License
GNU GPLv3

Copyrights (c) 2018 f1x.studio (Michal Szwaj)

*AndroidAuto is registered trademark of Google Inc.*

### Used software
 - [aasdk](https://github.com/f1xpl/aasdk)
 - [Boost libraries](http://www.boost.org/)
 - [Qt libraries](https://www.qt.io/)
 - [CMake](https://cmake.org/)
 - [RtAudio](https://www.music.mcgill.ca/~gary/rtaudio/playback.html)
 - Broadcom ilclient from RaspberryPI 3 firmware
 - OpenMAX IL API

### Building

OpenAuto provides a unified build script (`build.sh`) that works consistently across local and Docker environments.

#### Quick Start

```bash
# Build release version (recommended for production)
# Note: main/master branches default to release, other branches default to debug
./build.sh release --package --with-aasdk

# Build debug version with symbols (for development/debugging)
# Debug builds create packages with -dbg suffix (openauto-dbg)
./build.sh debug --package

# Auto-detect build type based on git branch
./build.sh --package

# Clean build
./build.sh release --clean --package
```

#### Build Script Options

```bash
Usage: ./build.sh [release|debug] [OPTIONS]

Build types:
  release        Build release version (default)
  debug          Build debug version with symbols

Options:
  --clean        Clean build directory before building
  --package      Create DEB packages after building
  --output-dir   Directory to copy packages (default: /output)
  --help         Show help message

Note: Builds are always done with NOPI=ON (no Pi-specific hardware)
```

#### Manual Building (Legacy)

If you need to build manually:

**AMD64/x86_64:**
1. Install dependencies from [prebuilts](https://github.com/opencardev/prebuilts) repository
2. Install Qt5 and development packages
3. Build with CMake:
   ```bash
   mkdir -p build-release
   cd build-release
   cmake -DCMAKE_BUILD_TYPE=Release ..
   make -j$(nproc)
   ```

   Note: The build script automatically enables NOPI=ON

**Raspberry Pi:**
1. Use the scripts in the `prebuilts` repository for `aasdk` and `openauto`
2. Or use the unified `build.sh` script:
   ```bash
   ./build.sh release --package
   ```

#### Docker Building

The Dockerfile uses the same `build.sh` script:

```bash
docker build -t openauto --build-arg DEBIAN_VERSION=trixie .
```

### Remarks
**This software is not certified by Google Inc. It is created for R&D purposes and may not work as expected by the original authors. Do not use while driving. You use this software at your own risk.**

## Testing

This project includes a comprehensive test suite to verify the functionality of the application.

### Running Tests

To run the tests, follow these steps:

1. Make sure you have built the project successfully
2. Navigate to the build directory and execute the test runner:

```bash
cd build
ctest -V
```

Or to run specific test categories:

```bash
cd build
# Run unit tests
./tests/unit/openauto_unit_tests

# Run integration tests
./tests/integration/openauto_integration_tests
```

### Test Coverage

You can generate test coverage reports using:

```bash
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DWITH_COVERAGE=ON ..
make
make test
make coverage
```

This will generate coverage reports in the `coverage` directory.

### Test Plan

For detailed information about test cases and validation procedures, refer to [TESTPLAN.md](TESTPLAN.md).
