# OpenAuto Date-Based Versioning Implementation

## 🎯 Overview

Successfully implemented a **date-based versioning system** for OpenAuto that automatically generates version numbers from the current date and includes git commit information for full traceability.

## 📅 Version Scheme

### Format: `YYYY.MM.DD+commit`

- **Major Version**: Year (e.g., 2025)
- **Minor Version**: Month (01-12)  
- **Patch Version**: Day (01-31)
- **Build Identifier**: Git commit hash (e.g., fc4e9d0)

### Example Versions
- `2025.07.20+fc4e9d0` - Built on July 20, 2025 from commit fc4e9d0
- `2025.12.25+a1b2c3d` - Built on December 25, 2025 from commit a1b2c3d

## 🔧 Implementation Details

### 1. CMakeLists.txt Changes

#### Version Detection
```cmake
# Date-based version components
string(TIMESTAMP OPENAUTO_BUILD_YEAR "%Y")      # Year as major version
string(TIMESTAMP OPENAUTO_BUILD_MONTH "%m")     # Month as minor version
string(TIMESTAMP OPENAUTO_BUILD_DAY "%d")       # Day as patch version

# Git information detection
execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD ...)
execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse HEAD ...)
execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --dirty --always ...)
execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD ...)
```

#### Compiler Definitions
```cmake
add_definitions(-DOPENAUTO_VERSION_MAJOR=${OPENAUTO_BUILD_MAJOR_RELEASE})
add_definitions(-DOPENAUTO_VERSION_MINOR=${OPENAUTO_BUILD_MINOR_RELEASE})
add_definitions(-DOPENAUTO_VERSION_PATCH=${OPENAUTO_BUILD_INCREMENTAL})
add_definitions(-DOPENAUTO_VERSION_STRING="${PROGRAM_VERSION_STRING}")
add_definitions(-DOPENAUTO_GIT_COMMIT="${OPENAUTO_GIT_COMMIT_SHORT}")
add_definitions(-DOPENAUTO_GIT_COMMIT_FULL="${OPENAUTO_GIT_COMMIT_FULL}")
add_definitions(-DOPENAUTO_GIT_BRANCH="${OPENAUTO_GIT_BRANCH}")
add_definitions(-DOPENAUTO_GIT_DESCRIBE="${OPENAUTO_GIT_DESCRIBE}")
```

### 2. Version Header File

Created `/include/modern/Version.hpp` with:
- Version constants accessible in C++ code
- Convenience functions for version strings
- Full build information formatting

```cpp
namespace openauto::modern {
    constexpr int MAJOR = OPENAUTO_VERSION_MAJOR;
    constexpr const char* VERSION_STRING = OPENAUTO_VERSION_STRING;
    std::string getFullVersionInfo();
    std::string getBuildInfo();
}
```

### 3. Package Integration

#### Package Descriptions
- Includes git commit information in package descriptions
- Version traceability: "Built from commit fc4e9d0 on branch crankshaft-ng_2025"

#### Package Names
- Release: `openauto-modern_2025.07.20+fc4e9d0_armhf.deb`
- Debug: `openauto-modern-debug_2025.07.20+fc4e9d0_armhf.deb`

## 🚀 Benefits

### 1. **Automatic Versioning**
- No manual version number management
- Builds automatically get unique, chronologically ordered versions
- No risk of version number conflicts or duplicates

### 2. **Full Traceability**
- Every package links to exact git commit
- Can reproduce any build from version number
- Easy to identify source code for any deployed version

### 3. **Chronological Ordering**
- Newer builds always have higher version numbers
- Package managers handle upgrades correctly
- Clear timeline of development progress

### 4. **Development Friendly**
- Debug builds get same version scheme
- Easy to correlate runtime issues with source code
- Git branch information available for context

## 🛠️ Usage

### Building with Version Information
```bash
# Build packages with automatic versioning
./build-packages.sh

# Check version information
./check-version.sh

# Version info is displayed during build:
# Version: 2025.07.20+fc4e9d0
# Branch: crankshaft-ng_2025
# Date: 2025-07-20 00:23:28
```

### Runtime Version Access
```cpp
#include "modern/Version.hpp"

// Get version components
int major = openauto::modern::MAJOR;        // 2025
int minor = openauto::modern::MINOR;        // 07
int patch = openauto::modern::PATCH;        // 20

// Get version strings
std::string version = openauto::modern::getVersionString();  // "2025.07.20+fc4e9d0"
std::string full = openauto::modern::getFullVersionInfo();   // Full info with branch
std::string build = openauto::modern::getBuildInfo();        // Complete build details
```

## 📦 Package Version Examples

### Release Package
```
Package: openauto-modern
Version: 2025.07.20+fc4e9d0
Architecture: armhf
Description: Modern Android Auto implementation for automotive systems
 Built from commit fc4e9d0 on branch crankshaft-ng_2025.
```

### Debug Package
```
Package: openauto-modern-debug  
Version: 2025.07.20+fc4e9d0
Architecture: armhf
Description: Modern Android Auto implementation for automotive systems (Debug Build)
 Built from commit fc4e9d0 on branch crankshaft-ng_2025.
```

## 🔄 Version Progression Examples

```
2025.07.20+fc4e9d0  ← Today's first build
2025.07.20+a1b2c3d  ← Today's second build (different commit)
2025.07.21+x9y8z7w  ← Tomorrow's build
2025.08.01+m5n6o7p  ← Next month's build
2026.01.01+q1w2e3r  ← Next year's build
```

## 🛡️ Robustness

### Git Repository Detection
- Graceful fallback when git is not available
- Shows "unknown" for git fields if not in a git repository
- Continues build process even without git information

### Build Date Consistency
- Date captured at CMake configure time
- All packages from same configure use same date
- Reconfigure needed to update version for new day

### Cross-Platform Support
- Works on Linux, macOS, Windows
- Uses CMake's built-in timestamp functions
- Git commands are platform-agnostic

## 📊 Implementation Summary

| Component | Status | Details |
|-----------|--------|---------|
| ✅ Date Detection | Complete | YYYY.MM.DD format |
| ✅ Git Integration | Complete | Commit hash, branch, describe |
| ✅ CMake Integration | Complete | Automatic version variables |
| ✅ Compiler Definitions | Complete | Available in C++ code |
| ✅ Package Integration | Complete | CPack uses new versioning |
| ✅ Header File | Complete | C++ API for version access |
| ✅ Build Scripts | Complete | Display version information |
| ✅ Documentation | Complete | Updated guides and examples |
| ✅ Validation Tools | Complete | Version testing scripts |

The date-based versioning system provides automatic, traceable, and chronologically ordered version numbers while maintaining full compatibility with the existing build and packaging system.
