# OpenAuto Package Build System

This directory contains the complete packaging system for OpenAuto, supporting both **Release** and **Debug** package builds.

## 🎯 Overview

The build system creates two distinct Debian packages:

- **`openauto-modern`** - Production release build optimized for performance
- **`openauto-modern-debug`** - Debug build with extensive logging and development tools

## 📦 Package Differences

### Release Package (`openauto-modern`)
- **Purpose**: Production deployment
- **Optimization**: `-O3` optimized build
- **Installation Path**: `/opt/openauto`
- **Configuration**: `/etc/openauto/`
- **Log Level**: INFO (minimal logging)
- **REST API Port**: 8080
- **Service Auto-start**: Yes
- **Security**: Hardened systemd settings
- **Dependencies**: Runtime libraries only

### Debug Package (`openauto-modern-debug`)
- **Purpose**: Development and troubleshooting
- **Optimization**: `-O0` with debug symbols
- **Installation Path**: `/opt/openauto-debug`
- **Configuration**: `/etc/openauto-debug/`
- **Log Level**: DEBUG (verbose logging)
- **REST API Port**: 8081
- **Service Auto-start**: No (manual start)
- **Security**: Relaxed for debugging
- **Dependencies**: Includes development tools (gdb, valgrind, strace)

## 🚀 Quick Start

### Build Both Packages
```bash
./build-packages.sh
```

### Build Release Only
```bash
./build-packages.sh --release-only
```

### Build Debug Only
```bash
./build-packages.sh --debug-only
```

### Validate Packages
```bash
./validate-packages.sh
```

## 📋 Build Process Details

### 1. Release Build Process
```bash
# Clean and configure
rm -rf build-release
mkdir build-release
cd build-release

# Configure for release
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build and package
make -j$(nproc)
make package
```

### 2. Debug Build Process
```bash
# Clean and configure
rm -rf build-debug
mkdir build-debug
cd build-debug

# Configure for debug with sanitizers
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON ..

# Build and package
make -j$(nproc)
make package
```

## 🏗️ Package Architecture

### Directory Structure
```
packaging/
├── config/                     # Configuration templates
│   ├── openauto.conf          # Release configuration
│   ├── openauto-debug.conf    # Debug configuration
│   ├── logger.conf            # Release logging config
│   ├── logger-debug.conf      # Debug logging config
│   ├── services.conf          # Release services config
│   └── services-debug.conf    # Debug services config
├── debian/                     # Package lifecycle scripts
│   ├── preinst                # Release pre-install
│   ├── preinst-debug          # Debug pre-install
│   ├── postinst               # Release post-install
│   ├── postinst-debug         # Debug post-install
│   ├── prerm                  # Release pre-remove
│   ├── prerm-debug            # Debug pre-remove
│   ├── postrm                 # Release post-remove
│   └── postrm-debug           # Debug post-remove
├── systemd/                    # SystemD service definitions
│   ├── openauto.service       # Release main service
│   ├── openauto-debug.service # Debug main service
│   ├── openauto-btservice.service        # Release bluetooth service
│   └── openauto-btservice-debug.service  # Debug bluetooth service
├── logrotate/                  # Log rotation configs
│   ├── openauto               # Release log rotation
│   └── openauto-debug         # Debug log rotation
├── udev/                       # USB device rules
│   └── 99-openauto.rules      # Shared USB permissions
└── scripts/                    # Utility scripts
    ├── openauto-setup.sh      # System setup script
    └── openauto-monitor.sh    # Monitoring script
```

## 🔧 Installation Paths

### Release Package Installation
```
/opt/openauto/                  # Binaries
├── autoapp                    # Main application
├── btservice                  # Bluetooth service
└── scripts/                   # Utility scripts

/etc/openauto/                  # Configuration
├── openauto.conf              # Main config
├── logger.conf               # Logging config
└── services.conf              # Services config

/var/lib/openauto/             # Runtime data
/var/log/openauto/             # Log files
```

### Debug Package Installation
```
/opt/openauto-debug/           # Debug binaries
├── autoapp                    # Debug build
├── btservice                  # Debug bluetooth service
└── scripts/                   # Utility scripts

/etc/openauto-debug/           # Debug configuration
├── openauto.conf              # Debug config
├── logger.conf               # Debug logging config
└── services.conf              # Debug services config

/var/lib/openauto-debug/       # Debug runtime data
├── cores/                     # Core dump location
└── data/                      # Debug data

/var/log/openauto-debug/       # Debug log files
```

## 🛠️ Package Management

### Installing Packages
```bash
# Install release package
sudo apt install ./packages/openauto-modern_*.deb

# Install debug package  
sudo apt install ./packages/openauto-modern-debug_*.deb
```

### Managing Services

#### Release Services
```bash
# Status
systemctl status openauto
systemctl status openauto-btservice

# Control
sudo systemctl start openauto
sudo systemctl stop openauto
sudo systemctl restart openauto

# Logs
journalctl -u openauto -f
journalctl -u openauto-btservice -f
```

#### Debug Services
```bash
# Status
systemctl status openauto-debug
systemctl status openauto-btservice-debug

# Control (manual start required)
sudo systemctl start openauto-debug
sudo systemctl stop openauto-debug
sudo systemctl restart openauto-debug

# Logs
journalctl -u openauto-debug -f
journalctl -u openauto-btservice-debug -f
```

## 🐛 Debug Features

### Core Dumps
Debug package automatically configures core dumps:
- **Location**: `/var/lib/openauto-debug/cores/`
- **Pattern**: `core.%e.%p.%t` (executable.pid.timestamp)
- **Analysis**: `gdb /opt/openauto-debug/autoapp /path/to/core`

### Memory Debugging
```bash
# Run with Valgrind
sudo -u openauto-debug valgrind --tool=memcheck --leak-check=full /opt/openauto-debug/autoapp

# Run with AddressSanitizer (built-in)
sudo systemctl start openauto-debug
```

### System Call Tracing
```bash
# Trace system calls
sudo -u openauto-debug strace -f -o /tmp/openauto-trace.log /opt/openauto-debug/autoapp
```

### Interactive Debugging
```bash
# Start with GDB
sudo -u openauto-debug gdb /opt/openauto-debug/autoapp

# In GDB:
(gdb) run --config /etc/openauto-debug/openauto.conf
(gdb) bt        # Backtrace on crash
(gdb) info registers
```

## 📊 Monitoring

### REST API Endpoints
- **Release**: `http://localhost:8080`
- **Debug**: `http://localhost:8081`

### Log Locations
- **Release**: `/var/log/openauto/openauto.log`
- **Debug**: `/var/log/openauto-debug/openauto-debug.log`

### System Integration
- **SystemD Services**: Auto-configured with proper dependencies
- **Log Rotation**: Automatic with configurable retention
- **USB Permissions**: udev rules for device access
- **User Management**: Dedicated system users with minimal privileges

## 🚨 Troubleshooting

### Build Issues
```bash
# Check dependencies
sudo apt update
sudo apt install build-essential cmake libboost-all-dev

# Clean and rebuild
./build-packages.sh --no-clean
```

### Package Issues
```bash
# Validate package contents
./validate-packages.sh

# Check package dependencies
dpkg-deb --info packages/openauto-modern_*.deb
```

### Runtime Issues
```bash
# Check service status
systemctl status openauto
systemctl status openauto-debug

# Check logs
journalctl -u openauto --since "1 hour ago"
journalctl -u openauto-debug --since "1 hour ago"

# Test REST API
curl http://localhost:8080/api/status
curl http://localhost:8081/api/status
```

## 🔄 Package Conflicts

The release and debug packages are mutually exclusive:
- Installing `openauto-modern-debug` will conflict with `openauto-modern`
- Installing `openauto-modern` will conflict with `openauto-modern-debug`
- This prevents accidentally running both simultaneously

## 📈 Package Updates

### Upgrading Packages
```bash
# Build new packages
./build-packages.sh

# Upgrade release
sudo apt install ./packages/openauto-modern_*.deb

# Upgrade debug
sudo apt install ./packages/openauto-modern-debug_*.deb
```

### Version Management
Package versions use **date-based semantic versioning**:
- **Major**: Year (e.g., 2025)
- **Minor**: Month (01-12)
- **Patch**: Day (01-31) 
- **Build**: Git commit hash (e.g., fc4e9d0)

Example: `2025.07.20+fc4e9d0`

This scheme provides:
- **Chronological ordering**: Newer builds have higher version numbers
- **Traceability**: Git commit ID links package to exact source code
- **Automatic versioning**: No manual version number management required

## 🎯 Production Deployment

For production systems, use the release package:

1. **Build release package**: `./build-packages.sh --release-only`
2. **Validate package**: `./validate-packages.sh`
3. **Install**: `sudo apt install ./packages/openauto-modern_*.deb`
4. **Verify**: `systemctl status openauto`
5. **Test**: `curl http://localhost:8080/api/status`

The release package provides optimized performance, minimal logging, and production-ready security settings.
