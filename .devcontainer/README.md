# AASDK Development Containers

This directory contains Docker-based development environments for building AASDK across multi## 🐛 Troubleshooting

### Container Won't Start
- Ensure Docker Desktop is running
- Check that you have sufficient disk space
- For ARM containers on x64 hosts, ensure QEMU emulation is enabled

### Podman Users
If you're using Podman instead of Docker:
1. Ensure Podman is running: `podman machine start`
2. The configurations use direct Dockerfile builds (not docker-compose) for better Podman compatibility
3. Platform-specific builds may require manual platform specification

### Build Fails
- Verify all dependencies are installed (run post-create script manually)
- Check that protobuf built successfully first
- Ensure correct target architecture is set

### Cross-Compilation Issues
- Verify cross-compilation toolchains are installed in the container
- Check that `TARGET_ARCH` environment variable is set correctly
- For ARMHF/ARM64, ensure the CMake toolchain file correctly detects compilerss.

## Available Configurations

### 1. Default (x64/amd64) - `devcontainer.json`
- **Platform**: linux/amd64
- **Use case**: Development on x64 machines, native builds
- **Dockerfile**: `Dockerfile.x64`

### 2. ARM64 - `devcontainer-arm64.json`  
- **Platform**: linux/arm64
- **Use case**: Building for ARM64 targets (e.g., Raspberry Pi 4, Apple Silicon)
- **Dockerfile**: `Dockerfile.arm64`

### 3. ARMHF - `devcontainer-armhf.json`
- **Platform**: linux/arm/v7
- **Use case**: Building for 32-bit ARM targets (e.g., Raspberry Pi 3, older ARM devices)
- **Dockerfile**: `Dockerfile.armhf`

## Quick Start

### Using VS Code

1. **Open the project in VS Code**
2. **Install the Dev Containers extension** if not already installed
3. **Choose your target architecture**:
   
   **For x64 (default):**
   - Press `Ctrl+Shift+P` (or `Cmd+Shift+P` on Mac)
   - Type "Dev Containers: Reopen in Container"
   - Select the command

   **For ARM64:**
   - Press `Ctrl+Shift+P` (or `Cmd+Shift+P` on Mac)  
   - Type "Dev Containers: Reopen in Container"
   - Choose "From 'devcontainer-arm64.json'"

   **For ARMHF:**
   - Press `Ctrl+Shift+P` (or `Cmd+Shift+P` on Mac)
   - Type "Dev Containers: Reopen in Container" 
   - Choose "From 'devcontainer-armhf.json'"

## Building the Project

Once inside any container, you can build the project using the provided scripts:

### Quick Build Scripts

```bash
# Build debug version
./build.sh debug

# Build release version  
./build.sh release

# Clean build (removes all build artifacts)
./build.sh debug clean
./build.sh release clean
```

### Create Packages

```bash
# Create release packages
./package.sh release

# Create debug packages
./package.sh debug
```

### Manual CMake Build

```bash
# Build protobuf first
cd protobuf/build
cmake -DCMAKE_BUILD_TYPE=Release -DTARGET_ARCH=$TARGET_ARCH ..
make -j$(nproc)
make install
cd ../..

# Build main project
mkdir -p build-release
cd build-release  
cmake -DCMAKE_BUILD_TYPE=Release -DTARGET_ARCH=$TARGET_ARCH ..
make -j$(nproc)
```

## Environment Variables

Each container sets the following environment variables:

- `TARGET_ARCH`: The target architecture (`amd64`, `arm64`, or `armhf`)
- `CC`: C compiler (set for cross-compilation when needed)
- `CXX`: C++ compiler (set for cross-compilation when needed)

## Cross-Compilation Details

### ARMHF Cross-Compilation
- Uses `arm-linux-gnueabihf-gcc` and `arm-linux-gnueabihf-g++`
- Automatically sets `TARGET_ARCH=armhf` in CMake

### ARM64 Cross-Compilation  
- Uses `aarch64-linux-gnu-gcc` and `aarch64-linux-gnu-g++`
- Automatically sets `TARGET_ARCH=arm64` in CMake

### x64 Native Compilation
- Uses system default GCC/G++
- Sets `TARGET_ARCH=amd64` in CMake

## VS Code Integration

All containers come pre-configured with:

- **C/C++ Extension Pack**: IntelliSense, debugging, code formatting
- **CMake Tools**: Integrated CMake support with build buttons
- **Additional tools**: JSON, YAML support, hex editor

### CMake Integration

The CMake Tools extension is configured to:
- Auto-configure on container startup
- Use separate build directories for debug/release (`build-debug`, `build-release`)
- Pass the correct `TARGET_ARCH` parameter automatically
- Provide build/debug buttons in the status bar

## Troubleshooting

### Container Won't Start
- Ensure Docker Desktop is running
- Check that you have sufficient disk space
- For ARM containers on x64 hosts, ensure QEMU emulation is enabled

### Build Fails
- Verify all dependencies are installed (run post-create script manually)
- Check that protobuf built successfully first
- Ensure correct target architecture is set

### Cross-Compilation Issues
- Verify cross-compilation toolchains are installed in the container
- Check that `TARGET_ARCH` environment variable is set correctly
- For ARMHF/ARM64, ensure the CMake toolchain file correctly detects compilers

## Container Architecture

```
.devcontainer/
├── devcontainer.json           # Default x64 configuration
├── devcontainer-arm64.json     # ARM64 configuration  
├── devcontainer-armhf.json     # ARMHF configuration
├── Dockerfile.x64              # x64 container definition
├── Dockerfile.arm64            # ARM64 container definition
├── Dockerfile.armhf            # ARMHF container definition
├── post-create.sh              # Post-creation setup script
└── README.md                   # This file
```

## Dependencies Included

All containers include:
- CMake 3.14+
- GCC/G++ build tools
- Git
- Protocol Buffers compiler and libraries
- Boost libraries (system, filesystem, thread, etc.)
- libusb development libraries
- OpenSSL development libraries
- Cross-compilation toolchains (where applicable)

## Performance Notes

- **x64**: Fastest build times, native execution
- **ARM64**: Good performance on Apple Silicon, emulated on x64  
- **ARMHF**: Slowest due to emulation on non-ARM hosts

For best performance when targeting ARM architectures, use an ARM-based host machine when possible.
