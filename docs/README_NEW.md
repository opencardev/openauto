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

#### [Troubleshooting Guide](troubleshooting-guide.md)
**Purpose**: Common problems and solutions
**Key Topics**:
- Build and compilation issues
- Runtime error resolution
- Modern component troubleshooting
- Performance optimization
- Debugging techniques

**Target Audience**: Developers, system administrators

## 🎯 Current Implementation Status

### ✅ Completed Components
- **Modern Logger System** - High-performance async logging with categories
- **Event Bus System** - Type-safe event publishing and subscription  
- **State Machine** - Centralized state management with validation
- **Configuration Manager** - JSON-based configuration with hot-reload
- **REST API Server** - OpenAPI 3.0 compliant with Swagger UI
- **Integration Layer** - Bridge between legacy and modern code
- **Legacy Migration** - All logging calls migrated to modern system

### 🏗️ Architecture Overview
The current implementation provides a modern foundation while maintaining compatibility with existing OpenAuto code. All components are optional and can be enabled/disabled via CMake flags.

### 📁 File Organization
```
include/modern/          # Modern component headers
src/modern/             # Modern component implementations  
docs/                   # Documentation
tests/                  # Unit and integration tests
```

## 🚀 Getting Started

### Build with Modern Components
```bash
# Configure with modern API enabled
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_MODERN_API=ON

# Build the project
cmake --build build --parallel

# Install (optional)
sudo cmake --install build
```

### Test the REST API
```bash
# Start OpenAuto with modern components
./build/autoapp

# In another terminal, test the API
curl http://localhost:8080/api/v1/health
curl http://localhost:8080/docs  # Swagger UI
```

### Use Modern Components in Code
```cpp
#include "modern/Logger.hpp"
#include "modern/EventBus.hpp"

// Modern logging
SLOG_INFO(GENERAL, "component", "Modern logging example");

// Event publishing
auto eventBus = std::make_shared<openauto::modern::EventBus>();
auto event = std::make_shared<openauto::modern::Event>(
    openauto::modern::EventType::UI_BUTTON_PRESSED, "main_ui");
eventBus->publish(event);
```

## 📖 Legacy Documentation Note

### Historical Planning Documents
Several documentation files in this directory contain planning information for future features that are not yet implemented:
- References to `backend/` and `frontend-qt6/` directories (don't exist)
- Theme system implementation (planned, not implemented)
- Advanced deployment scenarios (beyond current capabilities)

### Current vs. Planned
- **Current**: Modern components integrated with existing Qt-based OpenAuto
- **Planned**: Separate backend/frontend architecture with QT6 and themes

Focus on the **Implementation Summary** and **Integration Guide** for accurate information about current capabilities.

## 🔄 Maintenance and Updates

This documentation is maintained to reflect the current implementation status. When new features are added, the documentation will be updated accordingly.

### Key Principles
- Document only implemented features
- Provide working code examples
- Include build and test instructions
- Maintain clear migration paths

For the most current implementation details, always refer to:
1. `implementation-summary.md` - Current status
2. Source code in `src/modern/` and `include/modern/`
3. CMakeLists.txt for build configuration
4. Working test cases in `tests/`
