# OpenAuto Build Guide

This guide explains how to build OpenAuto with the enhanced versioning system and PNG profile checking features.

## Prerequisites

### Required Packages
Install the required dependencies:

```bash
# Core build dependencies
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    qtbase5-dev \
    qtmultimedia5-dev \
    libqt5multimedia5-plugins \
    qtdeclarative5-dev \
    qtquickcontrols2-5-dev \
    libboost-all-dev \
    libprotobuf-dev \
    protobuf-compiler \
    librtaudio-dev \
    libasound2-dev \
    libpulse-dev

# PNG checking and fixing tools (for automatic PNG profile fixes)
sudo apt-get install -y pngcheck pngcrush imagemagick

# Coverage tools (for coverage reporting - gcov comes with gcc)
sudo apt-get install -y lcov

# Testing framework (for running unit and integration tests)
sudo apt-get install -y libgtest-dev
# Build GoogleTest from source
cd /usr/src/googletest && sudo cmake . && sudo make && sudo cp lib/* /usr/lib/
# Copy headers to system include directory
sudo cp -r /usr/src/googletest/googlemock/include/gmock /usr/include/
sudo cp -r /usr/src/googletest/googletest/include/gtest /usr/include/
```

### AASDK Library
You need to have AASDK installed. If using system packages:

```bash
# If AASDK is installed system-wide
export AASDK_INCLUDE_DIRS=/usr/include/aasdk
export AASDK_LIBRARIES=/usr/lib/libaasdk.so
```

## Build Configuration Options

### Basic Release Build (Recommended)
```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DNOPI=true \
      -DAASDK_INCLUDE_DIRS=/usr/include/aasdk \
      ..
```

### Debug Build with Coverage
```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DWITH_COVERAGE=ON \
      -DNOPI=true \
      -DAASDK_INCLUDE_DIRS=/usr/include/aasdk \
      ..
```

### Raspberry Pi Build
```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DAASDK_INCLUDE_DIRS=/usr/include/aasdk \
      ..
```

## Building the Project

### Standard Build
```bash
# From the project root directory
cd /home/pi/openauto_ori && cmake --build build -j$(($(nproc) - 1))
```

### Clean Build
```bash
# Clean previous build
rm -rf build
mkdir build
cd build

# Configure
cmake -DCMAKE_BUILD_TYPE=Release -DNOPI=true -DAASDK_INCLUDE_DIRS=/usr/include/aasdk ..

# Build
cmake --build . -j$(($(nproc) - 1))
```

## Build Features

### Automatic Versioning
The build system now includes automatic versioning based on git:
- **Format**: `YYYY.MM.DD+gitcommit` (e.g., `2025.10.06+a788614`)
- **Git Integration**: Automatically captures branch, commit, and build date
- **Version Display**: Version information is embedded in the compiled binaries

### PNG Profile Checking
During configuration, the build system automatically:
- ✅ Scans all PNG files in the assets directory
- 🔧 Detects problematic iCCP profiles that cause libpng warnings
- 🛠️ Automatically fixes PNG files using `pngcrush` or `imagemagick`
- 📊 Reports summary of files checked and fixed

### Build Targets

#### Main Applications
- **autoapp**: Main OpenAuto application
- **btservice**: Bluetooth service component

#### Available Targets
- **autoapp**: Main OpenAuto application
- **btservice**: Bluetooth service component
- **unit_tests**: Unit test suite (builds with memory-safe compilation)

#### Building Specific Targets
```bash
# Build main application only
cmake --build build --target autoapp -j2

# Build unit tests (recommended approach for limited memory systems)
cmake --build build --target unit_tests -j1

# Build bluetooth service
cmake --build build --target btservice -j2
```

## Testing

### Test Suite Status
This project includes unit and integration tests in the `/tests` directory, however:
- **Main Build**: ✅ **Successful** - All core functionality builds and works
- **Test Status**: ⚠️ **Needs Re-implementation** - Tests require API updates for current aasdk version
- **Test Framework**: ✅ GoogleTest infrastructure is properly configured

### Current Test Issues Requiring Fixes
The existing tests have several compatibility issues that need addressing:

1. **API Compatibility**: Tests written for older aasdk API version
   - `AccessoryModeQueryFactory` → needs current factory interfaces
   - USB wrapper constructors have changed parameters
   - Bluetooth device API methods have evolved

2. **Missing Dependencies**: Some tests expect features not in current build
   - Missing mock implementations for USB/TCP wrappers
   - Incomplete MockConfiguration class
   - Missing signal/slot test infrastructure

3. **Qt Integration**: UI tests need Qt Test framework updates
   - QSignalSpy usage needs proper setup
   - Widget testing requires display environment

### Recommended Approach for Test Development

#### Phase 1: Basic Unit Tests (Easiest)
Start with simple unit tests that don't depend on complex APIs:
```bash
# Focus on these first:
- ConfigurationTests.cpp (configuration validation)
- Basic service tests without USB/Bluetooth dependencies
```

#### Phase 2: Mock Implementation Updates
Update mock classes to match current APIs:
```bash
# Files needing updates:
- tests/mocks/MockConfiguration.hpp (incomplete)
- tests/mocks/MockAndroidAutoEntity.hpp (API changes)
- Add missing USB/TCP wrapper mocks
```

#### Phase 3: Integration Tests
Fix integration tests with current API signatures:
```bash
# Major updates needed:
- AndroidAutoIntegrationTests.cpp (USB wrapper API)
- BluetoothIntegrationTests.cpp (Bluetooth API)
- UIIntegrationTests.cpp (Qt Test setup)
```

### Building Without Tests (Current Recommendation)
For production use, build without tests:
```bash
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF -DNOPI=true -DAASDK_INCLUDE_DIRS=/usr/include/aasdk ..
cmake --build . -j$(($(nproc) - 1))
```

### Test Development Environment Setup
If you want to work on fixing the tests:
```bash
sudo apt-get install -y libgtest-dev qtbase5-dev-tools
cd /usr/src/googletest && sudo cmake . && sudo make && sudo cp lib/* /usr/lib/
# Copy headers to system include directory
sudo cp -r /usr/src/googletest/googlemock/include/gmock /usr/include/
sudo cp -r /usr/src/googletest/googletest/include/gtest /usr/include/
```

### Running Tests (Once Fixed)
When tests are updated, they can be run with:
```bash
cd build
# Configure with tests enabled
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DNOPI=true -DAASDK_INCLUDE_DIRS=/usr/include/aasdk ..
cmake --build . -j$(($(nproc) - 1))

# Run all tests
ctest -V

# Or run specific test categories
./tests/unit_tests
./tests/integration_tests
```

### Generate Coverage Report
For coverage reporting (main application execution):
```bash
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DWITH_COVERAGE=ON -DBUILD_TESTS=OFF -DNOPI=true -DAASDK_INCLUDE_DIRS=/usr/include/aasdk ..
cmake --build . -j$(($(nproc) - 1))
# Run application to generate coverage data, then:
cmake --build . --target coverage
```

### Test Re-implementation Guide

For developers wanting to fix the test suite, here's a systematic approach:

#### Step 1: Fix MockConfiguration.hpp
```bash
# Current issues:
- Missing QString #include <QString>
- Incomplete method implementations
- Missing getters/setters for all configuration options

# Required methods to implement:
- All getters/setters from the Configuration interface
- Proper QString handling
- Mock behavior for configuration validation
```

#### Step 2: Create Missing Mock Classes
```bash
# Missing mocks that tests expect:
- MockUSBWrapper (for USB device simulation)
- MockTCPWrapper (for network simulation)  
- MockUSBHub (for USB hub management)
- MockConnectivityManager (for connection handling)

# These should inherit from their respective interfaces
```

#### Step 3: Update API Calls
```bash
# Fix API compatibility in:
- AndroidAutoIntegrationTests.cpp: 
  * AccessoryModeQueryFactory → correct factory pattern
  * USB wrapper constructor parameters
  * Device handle management

- BluetoothIntegrationTests.cpp:
  * LocalBluetoothDevice API methods
  * QBluetoothDevice API updates
  * Signal/slot connections

- UIIntegrationTests.cpp:
  * QTest framework setup
  * Widget testing environment
  * QSignalSpy proper usage
```

#### Step 4: Test Environment Setup
```bash
# For UI tests that need display:
export QT_QPA_PLATFORM=offscreen

# For tests requiring specific permissions:
sudo usermod -a -G dialout $USER  # for USB device access
```

## Installation

### Install to System
```bash
cd build
sudo cmake --install .
```

### Create Packages (if enabled)
```bash
cd build
cpack
```

## Troubleshooting

### PNG Tools Missing
If you see warnings about missing PNG tools:
```bash
sudo apt-get install -y pngcheck pngcrush imagemagick
```

### AASDK Not Found
Make sure AASDK is properly installed and set the correct paths:
```bash
cmake -DAASDK_INCLUDE_DIRS=/path/to/aasdk/include \
      -DAASDK_LIBRARIES=/path/to/aasdk/lib/libaasdk.so \
      ..
```

### Git Information Missing
If version shows as "unknown", ensure you're building from a git repository:
```bash
git status  # Should show current branch
```

### Out of Memory (OOM) Build Failures
If compilation fails with "Killed signal terminated program cc1plus":

**Immediate Fix:**
```bash
# Use single-threaded compilation
cd build
make -j1 unit_tests  # or any other target
```

**Long-term Solutions:**
```bash
**Long-term Solutions:**
```bash
# 1. Use the safe build script
./scripts/build_safe.sh unit_tests

# 2. Check system memory and adjust accordingly
free -h
# If available memory < 2GB, always use -j1
```
# If available memory 2-4GB, use -j2
# If available memory > 4GB, use -j4 or higher

# 3. Monitor memory during build
watch -n 1 'free -h && ps aux | grep cc1plus | wc -l'

# 4. Check system logs for OOM events
dmesg | tail -20 | grep -i "killed\|oom"
```

**Prevention:**
- Always use limited parallel jobs on Raspberry Pi: `-j1` or `-j2`
- Close unnecessary applications before building
- Consider increasing swap space for large compilations

### Build Performance and Memory Management

The build uses `$(($(nproc) - 1))` cores by default, leaving one core free for system responsiveness. However, on systems with limited RAM (4GB or less), parallel compilation can cause out-of-memory (OOM) errors.

#### Memory-Safe Building (Recommended for Raspberry Pi)

For systems with limited memory, use these approaches:

**Option 1: Limited Parallel Jobs (Recommended)**
```bash
# For 4GB systems, use at most 2 parallel jobs
cmake --build build -j2

# For 2GB systems, use single-threaded builds
cmake --build build -j1
```

**Option 2: Use the Safe Build Script**
A safe build script has been created that automatically handles memory limitations:
```bash
# Use the safe build script (automatically tries -j2, falls back to -j1)
./scripts/build_safe.sh unit_tests  # or any other target
```

**Option 3: Manual Memory-Aware Building**
```bash
# Check available memory before building
free -h

# If available memory < 2GB, use single thread
if [ $(free -m | awk 'NR==2{print $7}') -lt 2000 ]; then
    cmake --build build -j1
else
    cmake --build build -j2
fi
```

#### Symptoms of Memory Issues
If you encounter these errors, reduce parallel jobs:
```
c++: fatal error: Killed signal terminated program cc1plus
compilation terminated.
make[3]: *** [CMakeFiles/target.dir/build.make:254: CMakeFiles/target.dir/file.cpp.o] Error 1
```

#### Memory Optimization Tips
- **Monitor memory usage**: `watch -n 1 free -h`
- **Check for OOM kills**: `dmesg | grep -i "killed\|oom"`
- **Increase swap if needed**: Create additional swap space for temporary relief
- **Close unnecessary applications** during compilation

#### Alternative Build Configurations
```bash
# Use all cores (only for systems with 8GB+ RAM)
cmake --build build -j$(nproc)

# Use specific number of cores based on your system
cmake --build build -j4  # For 8GB+ systems
cmake --build build -j2  # For 4GB systems  
cmake --build build -j1  # For 2GB systems or if memory issues persist
```

### CMake Warnings
If you see CMake policy warnings, they have been addressed in the updated CMakeLists.txt files. Make sure you're using the latest version of the project files.

## Build Output

After successful build, you'll find:
- **Executables**: `bin/autoapp`, `bin/btservice`
- **Libraries**: `lib/` directory
- **Version Info**: Displayed during build and embedded in binaries
- **PNG Report**: Shows any PNG files that were fixed

The version information is embedded in the binaries and displayed during the build process.