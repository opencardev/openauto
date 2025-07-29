# OpenAuto Documentation

## Overview

This documentation covers the current OpenAuto implementation with its modernized architecture components. The modernization includes a new logger system, event bus, state machine, REST API server, and eliminates legacy `/tmp` file dependencies.

## 📋 Quick Start

### Essential Reading Order
1. **[Implementation Summary](implementation-summary.md)** - Current implementation status
2. **[Build Guide](build-guide.md)** - Build the system
3. **[Integration Guide](integration-guide.md)** - Use modern components
4. **[API Documentation](api-documentation.md)** - REST API reference
5. **[Troubleshooting Guide](troubleshooting-guide.md)** - Solve problems

### Developer Resources
1. **[Logger Migration Guide](logger-migration.md)** - Modern logging system
2. **[Version Scheme](version-scheme-and-vscode.md)** - Development environment

## 📚 Documentation Structure

### Current Implementation

#### [Implementation Summary](implementation-summary.md)
**Purpose**: Complete overview of current implementation status
**Key Topics**:
- Modern architecture components (Logger, EventBus, StateMachine, REST API)
- Legacy system migration status
- Build system integration
- Current capabilities and features

**Target Audience**: Developers, project managers

#### [Integration Guide](integration-guide.md)
**Purpose**: How to use the modern components in your code
**Key Topics**:
- Modern event bus usage
- State machine integration
- Configuration management
- REST API integration examples

**Target Audience**: Software developers

### Build and Development

#### [Build Guide](build-guide.md)
**Purpose**: Complete build instructions for all platforms
**Key Topics**:
- Dependencies and prerequisites
- Platform-specific builds (Linux, Windows, Raspberry Pi)
- Modern API compilation flags
- Testing and verification

**Target Audience**: Developers, build engineers

#### [Version Scheme and VS Code Integration](version-scheme-and-vscode.md)
**Purpose**: Development environment and versioning system
**Key Topics**:
- Date-based versioning system (YYYY.MM.DD+commit)
- Git integration and version tracking
- VS Code tasks configuration
- Build automation
- Development workflow optimization

**Target Audience**: Developers, build engineers

### Technical References

#### [Logger Migration Guide](logger-migration.md)
**Purpose**: Detailed guide for the modern logging system
**Key Topics**:
- Modern logging patterns
- Migration from legacy logging
- Performance benefits
- Configuration and usage
- Troubleshooting

**Target Audience**: Software developers

#### [API Documentation](api-documentation.md)
**Purpose**: Complete REST API reference
**Key Topics**:
- All API endpoints with examples
- Authentication and security
- Request/response formats
- Error handling
- Integration examples

**Target Audience**: Application developers, integration partners

#### [REST API Quick Reference](rest-api-quick-reference.md)
**Purpose**: Fast reference for common API operations and debugging
**Key Topics**:
- Essential API endpoints and examples
- Authentication and error handling
- Testing commands and debugging tips
- Configuration examples and status codes

**Target Audience**: All developers and users

#### [Troubleshooting Guide](troubleshooting-guide.md)
**Purpose**: Comprehensive problem-solving resource
**Key Topics**:
- Diagnostic tools and scripts
- Common issues and solutions
- Performance debugging
- Service recovery procedures
- Log analysis techniques
- Hardware-specific troubleshooting

**Target Audience**: System administrators, support engineers, developers

### Status and Progress

#### [Migration Status](migration-status.md)
**Purpose**: Track migration progress and identify remaining work
**Key Topics**:
- Component migration status
- File migration tracking
- Build system updates
- Known issues and workarounds

**Target Audience**: Project managers, developers

## 🛠 Quick Reference

### Key Components

| Component | Purpose | Documentation |
|-----------|---------|---------------|
| **Modern Logger** | High-performance, categorized logging | [Logger Migration Guide](logger-migration.md) |
| **Event Bus** | Decoupled inter-component communication | [Integration Guide](integration-guide.md) |
| **State Machine** | Centralized state management | [Integration Guide](integration-guide.md) |
| **REST API** | Remote control and monitoring | [REST API Comprehensive Guide](rest-api-comprehensive.md) |
| **Configuration Manager** | Dynamic configuration management | [Integration Guide](integration-guide.md) |

### REST API Quick Start

| Task | Command | Documentation |
|------|---------|---------------|
| **Start API Server** | `./autoapp --enable-api` | [REST API Comprehensive Guide](rest-api-comprehensive.md) |
| **Test Health** | `curl http://localhost:8080/api/health` | [REST API Testing Guide](rest-api-testing.md) |
| **View API Docs** | Open `http://localhost:8080/docs` | [REST API Implementation Guide](rest-api-implementation.md) |
| **Run API Tests** | `./test_rest_api.sh` | [REST API Testing Guide](rest-api-testing.md) |
| **Debug API Issues** | Check logs + network | [REST API Debugging Guide](rest-api-debugging.md) |
| **Quick Reference** | All common commands | [REST API Quick Reference](rest-api-quick-reference.md) |

### Build Targets

| Platform | Build Command | Documentation |
|----------|---------------|---------------|
| **Standard Linux** | `cmake -DENABLE_MODERN_API=ON ..` | [Build Guide](build-guide.md#standard-build) |
| **Raspberry Pi** | `cmake -DRPI_BUILD=ON -DENABLE_MODERN_API=ON ..` | [Build Guide](build-guide.md#raspberry-pi-build) |
| **Docker** | `docker build -t openauto:latest .` | [Build Guide](build-guide.md#docker-build) |
| **Development** | `cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_MODERN_API=ON ..` | [Build Guide](build-guide.md#development-build) |

### Common Commands

#### Build Commands
```bash
# Clean build
rm -rf build && mkdir build && cd build

# Configure with modern API
cmake -DENABLE_MODERN_API=ON ..

# Build with all cores
make -j$(nproc)

# Run tests
ctest --verbose
```

#### Service Management
```bash
# Start services
sudo systemctl start openauto openauto-btservice

# Check status
sudo systemctl status openauto

# View logs
sudo journalctl -u openauto -f

# Restart services
sudo systemctl restart openauto
```

#### API Testing
```bash
# Check system status
curl http://localhost:8080/api/v1/status

# Get recent logs
curl "http://localhost:8080/api/v1/logs?limit=10&level=ERROR"

# Send test event
curl -X POST http://localhost:8080/api/v1/events \
  -H "Content-Type: application/json" \
  -d '{"type":"TEST","category":"USER","data":{"test":true}}'
```

#### Debugging
```bash
# Run diagnostic script
./scripts/diagnose.sh

# Monitor logs in real-time
tail -f /var/log/openauto/*.log

# Check modern API status
curl http://localhost:8080/api/v1/health
```

## 🔧 Configuration Files

### Key Configuration Locations

| File | Purpose | Example |
|------|---------|---------|
| `/etc/openauto/openauto.conf` | Main configuration | [Deployment Guide](deployment-guide.md#main-configuration) |
| `/etc/openauto/logger.conf` | Logger settings | [Logger Migration Guide](logger-migration.md#configuration) |
| `/etc/systemd/system/openauto.service` | Service definition | [Deployment Guide](deployment-guide.md#systemd-services) |

### Essential Settings

```ini
# Enable modern architecture
[modern_api]
enable_rest_api = true
enable_event_bus = true
enable_state_machine = true

# Configure logging
[logging]
async_logging = true
file_output = true
console_output = true

# Android Auto settings
[android_auto]
usb_hotplug = true
wifi_projection = true
```

## 🐛 Common Issues

### Build Issues
- **Missing dependencies**: See [Build Guide - Dependencies](build-guide.md#required-dependencies)
- **CMake errors**: See [Build Guide - Troubleshooting](build-guide.md#troubleshooting-build-issues)
- **Legacy file conflicts**: See [Troubleshooting Guide - Legacy Conflicts](troubleshooting-guide.md#legacy-file-conflicts)

### Runtime Issues
- **Service won't start**: See [Troubleshooting Guide - Service Issues](troubleshooting-guide.md#service-startup-issues)
- **API not responding**: See [Troubleshooting Guide - REST API](troubleshooting-guide.md#rest-api-not-responding)
- **No audio/video**: See [Troubleshooting Guide - Hardware Issues](troubleshooting-guide.md#audio-issues)

### Performance Issues
- **High CPU usage**: See [Troubleshooting Guide - Performance](troubleshooting-guide.md#high-cpu-usage)
- **Memory leaks**: See [Troubleshooting Guide - Memory](troubleshooting-guide.md#high-memory-usage)
- **Log performance**: See [Logger Migration Guide - Performance](logger-migration.md#performance-benefits)

## 📊 Migration Checklist

### Pre-Migration
- [ ] Read [Modernization Summary](modernization-summary.md)
- [ ] Review [Build Guide](build-guide.md) requirements
- [ ] Check [Migration Status](migration-status.md) for known issues

### During Migration
- [ ] Follow [Logger Migration Guide](logger-migration.md) steps
- [ ] Update build system per [Build Guide](build-guide.md)
- [ ] Test modern components using [Integration Guide](integration-guide.md)

### Post-Migration
- [ ] Deploy using [Deployment Guide](deployment-guide.md)
- [ ] Test API endpoints from [API Documentation](api-documentation.md)
- [ ] Verify with [Troubleshooting Guide](troubleshooting-guide.md) checks

## 🚀 Getting Started

### For New Users
1. Read the [Modernization Summary](modernization-summary.md) to understand the architecture
2. Follow the [Build Guide](build-guide.md) to compile the system
3. Use the [Deployment Guide](deployment-guide.md) for installation
4. Test with examples from [API Documentation](api-documentation.md)

### For Existing Users
1. Check [Migration Status](migration-status.md) for compatibility
2. Follow [Logger Migration Guide](logger-migration.md) to update code
3. Update build system using [Build Guide](build-guide.md)
4. Redeploy using [Deployment Guide](deployment-guide.md)

### For Developers
1. Review [Integration Guide](integration-guide.md) for modern patterns
2. Use [API Documentation](api-documentation.md) for external integration
3. Reference [Logger Migration Guide](logger-migration.md) for logging best practices
4. Keep [Troubleshooting Guide](troubleshooting-guide.md) handy for debugging

## 📞 Support and Resources

### Documentation Updates
- All documentation is located in the `docs/` directory
- Markdown format for easy editing and version control
- Cross-references between documents for navigation

### Additional Resources
- **Source Code**: Modern components in `src/modern/` and `include/modern/`
- **Examples**: API usage examples in each documentation file
- **Scripts**: Diagnostic and migration scripts in `scripts/` directory

### Getting Help
1. Check the relevant troubleshooting section first
2. Review API documentation for endpoint usage
3. Use diagnostic scripts for system analysis
4. Follow build guide for compilation issues

This comprehensive documentation suite provides everything needed to understand, build, deploy, and maintain OpenAuto's modern architecture. Start with the [Modernization Summary](modernization-summary.md) and follow the recommended reading order for your use case.
