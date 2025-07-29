# OpenAuto Documentation - Current Implementation Guide

## 📋 Current Documentation Structure

The OpenAuto documentation has been reorganized to focus on **implemented features only**. Here's what's available for current development:

### Core Documentation (Current Implementation)

| Document | Purpose | Status |
|----------|---------|---------|
| **[README.md](README.md)** | Documentation index and getting started | ✅ Current |
| **[implementation-summary.md](implementation-summary.md)** | Complete overview of implemented features | ✅ Current |
| **[modernization-summary.md](modernization-summary.md)** | What has been modernized | ✅ Current |
| **[integration-guide.md](integration-guide.md)** | How to use modern components | ✅ Current |

### Development & Build

| Document | Purpose | Status |
|----------|---------|---------|
| **[build-guide.md](build-guide.md)** | Build instructions for all platforms | ✅ Current |
| **[version-scheme-and-vscode.md](version-scheme-and-vscode.md)** | Development environment setup | ✅ Current |
| **[test-guide.md](test-guide.md)** | Testing procedures | ✅ Current |

### Technical References

| Document | Purpose | Status |
|----------|---------|---------|
| **[api-documentation.md](api-documentation.md)** | REST API reference | ✅ Current |
| **[logger-migration.md](logger-migration.md)** | Modern logging system | ✅ Current |
| **[troubleshooting-guide.md](troubleshooting-guide.md)** | Problem solving | ✅ Current |

### Archived Documentation

| Directory | Contents | Purpose |
|-----------|----------|---------|
| **`archive/planning/`** | Future feature plans | Reference only |
| **`archive/legacy/`** | Previous documentation versions | Historical reference |

## 🎯 What's Actually Implemented

### ✅ Working Components

1. **Modern Logger System** - High-performance async logging
2. **Event Bus System** - Type-safe publish/subscribe events  
3. **State Machine** - Centralized state management
4. **Configuration Manager** - Dynamic JSON configuration
5. **REST API Server** - OpenAPI 3.0 compliant HTTP API
6. **Integration Layer** - Bridge between legacy and modern code

### 🚀 Quick Start

```bash
# Build with modern components
cmake -S . -B build -DENABLE_MODERN_API=ON
cmake --build build --parallel

# Run with REST API
./build/autoapp

# Test API (in another terminal)
curl http://localhost:8080/api/v1/health
curl http://localhost:8080/docs  # Swagger UI
```

### 💻 Code Integration

```cpp
#include "modern/Logger.hpp"
#include "modern/EventBus.hpp"

// Modern logging
SLOG_INFO(GENERAL, "component", "Application started");

// Event handling
auto eventBus = std::make_shared<openauto::modern::EventBus>();
eventBus->subscribe(openauto::modern::EventType::UI_BUTTON_PRESSED,
    [](const auto& event) {
        SLOG_DEBUG(UI, "handler", "Button pressed: {}",
                   event->getData("button_id"));
    });
```

## 📁 Architecture Overview

### Current Structure
```
OpenAuto (Qt-based application)
├── src/modern/              # Modern component implementations
├── include/modern/          # Modern component headers
├── tests/                   # Unit and integration tests
├── docs/                    # Current implementation docs
└── CMakeLists.txt          # Build configuration
```

### Build Integration
- **CMake Flag**: `-DENABLE_MODERN_API=ON` (default)
- **Dependencies**: nlohmann/json, cpp-httplib
- **Optional**: Modern components can be disabled
- **Compatible**: Works with existing OpenAuto features

## 🔗 Key Integration Points

### REST API Endpoints
- `GET /api/v1/health` - System health check
- `GET /api/v1/events` - Event history
- `POST /api/v1/events` - Publish event
- `GET /api/v1/state` - Current system state
- `GET /api/v1/config` - Configuration access
- `GET /docs` - Interactive Swagger UI

### Event Types
- System: `SYSTEM_STARTUP`, `SYSTEM_SHUTDOWN`
- Android Auto: `ANDROID_AUTO_CONNECTED`, `ANDROID_AUTO_DISCONNECTED`
- UI: `UI_BUTTON_PRESSED`, `UI_SCREEN_CHANGED`
- Media: `MEDIA_PLAY`, `MEDIA_PAUSE`, `MEDIA_TRACK_CHANGED`
- Camera: `CAMERA_RECORDING_STARTED`, `CAMERA_PHOTO_TAKEN`

### Log Categories
- `GENERAL`, `SYSTEM`, `ANDROID_AUTO`, `UI`, `CAMERA`
- `NETWORK`, `MEDIA`, `CONFIG`, `API`, `STATE`

## 🛠️ Development Workflow

### VS Code Tasks
OpenAuto includes 18 specialized VS Code tasks for development automation:
- **Build Release Package** - Production builds
- **Build Debug Package** - Development builds  
- **Run Tests** - Execute test suite
- **Full Build and Test Pipeline** - Complete CI/CD

See [version-scheme-and-vscode.md](version-scheme-and-vscode.md) for complete task documentation.

### Testing
```bash
# Run unit tests
ctest --test-dir build/tests/unit

# Run integration tests  
ctest --test-dir build/tests/integration

# Test API endpoints
curl http://localhost:8080/api/v1/health
```

## 📖 Documentation Principles

### Current Focus
- ✅ Document only implemented features
- ✅ Provide working code examples
- ✅ Include build and test instructions
- ✅ Maintain clear migration paths

### Archived Content
- 📁 Planning documents in `archive/planning/`
- 📁 Old versions in `archive/legacy/`
- 🔍 Reference for future development
- ❌ Not applicable to current development

## 🎯 For Developers

### New to OpenAuto
1. Start with [README.md](README.md)
2. Follow [build-guide.md](build-guide.md)
3. Read [integration-guide.md](integration-guide.md)
4. Test with [api-documentation.md](api-documentation.md)

### Existing Contributors
1. Check [implementation-summary.md](implementation-summary.md)
2. Review [modernization-summary.md](modernization-summary.md)
3. Update logging with [logger-migration.md](logger-migration.md)
4. Use [troubleshooting-guide.md](troubleshooting-guide.md) for issues

### Integration Partners
1. Use [api-documentation.md](api-documentation.md) for REST API
2. See [integration-guide.md](integration-guide.md) for examples
3. Test with Swagger UI at `/docs` endpoint

This documentation structure ensures developers have accurate, current information for working with OpenAuto's modern architecture while keeping future planning materials available for reference.
