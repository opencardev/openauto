# OpenAuto Development Container

This development container provides a complete environment for building and developing OpenAuto.

## Features

- **Complete build environment** with all dependencies pre-installed
- **aasdk packages** automatically installed from the `packages/` folder
- **18 VSCode tasks** for comprehensive build/test/deploy automation
- **Debugging support** with GDB integration
- **Test environment** with GTest and coverage tools
- **Qt5 development** with all required modules

## What's Included

### Build Tools
- GCC/G++ compiler
- CMake and Ninja build systems
- pkg-config
- ccache for faster compilation

### Libraries & Dependencies
- Qt5 (Core, Widgets, Multimedia, Bluetooth, Network, DBus)
- Boost libraries (all required components)
- Protocol Buffers
- USB development libraries
- Audio libraries (RtAudio, TagLib)
- GStreamer multimedia framework

### Development Tools
- GDB debugger
- Valgrind memory checker
- lcov/gcovr for coverage analysis
- Git version control

### Pre-installed Packages
- libaasdk-amd64 (runtime)
- libaasdk-amd64-dev (development headers)

## VSCode Integration

The container includes 18 specialized tasks:

### Build Tasks
- **Build Release Package** (default build task)
- **Build Debug Package** 
- **Build Both Packages**
- **Configure Release/Debug Build**
- **Build Release/Debug (CMake)**
- **Clean All Builds**

### Test Tasks
- **Run Tests** (default test task)
- **Run Tests (Verbose)**
- **Validate Packages**
- **Check Version Info**

### Deployment Tasks
- **Install Release/Debug Package**
- **Start OpenAuto Service**
- **Check Service Status**
- **View Service Logs**

### Pipeline Task
- **Full Build and Test Pipeline** (complete CI/CD workflow)

## Quick Start

1. Open the workspace in VSCode
2. When prompted, click "Reopen in Container"
3. Wait for the container to build and start
4. Run tasks using `Ctrl+Shift+P` → "Tasks: Run Build Task"

## File Structure

```
.devcontainer/
├── devcontainer.json      # Container configuration
├── Dockerfile.x64         # Container image definition
├── post-create.sh         # Post-creation setup script
└── README.md             # This file

.vscode/
├── tasks.json            # 18 VSCode tasks
├── launch.json           # Debug configurations
├── settings.json         # Workspace settings
└── c_cpp_properties.json # IntelliSense configuration
```

## Environment Variables

- `TARGET_ARCH=amd64` - Target architecture
- `CMAKE_BUILD_PARALLEL_LEVEL=4` - Default parallel build jobs
- `CCACHE_DIR=/tmp/ccache` - Compilation cache directory

## Debugging

The container includes debug configurations for:
- Main OpenAuto application (`autoapp`)
- Unit tests (`openauto_unit_tests`)

Use `F5` to start debugging or select from the debug dropdown.

## Troubleshooting

If you encounter issues:

1. **Rebuild container**: `Ctrl+Shift+P` → "Dev Containers: Rebuild Container"
2. **Check post-create script**: View terminal output during container creation
3. **Verify packages**: Ensure aasdk .deb files exist in `packages/` directory
4. **Check dependencies**: Run `apt list --installed | grep aasdk`

## Documentation

- [Build Guide](../docs/build-guide.md)
- [Development Environment Summary](../docs/DEV_ENVIRONMENT_SUMMARY.md)
- [VSCode Tasks Guide](../docs/version-scheme-and-vscode.md)
