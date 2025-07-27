# OpenAuto Version Scheme and Development Environment

## Date-Based Versioning System (YYYY.MM.DD+commit)

OpenAuto uses a modern date-based versioning scheme that provides clear, chronological version identification with git integration.

### Version Format: `YYYY.MM.DD+commit`

- **YYYY** (Major): 4-digit year (e.g., 2025)
- **MM** (Minor): 2-digit month with leading zero (01-12)  
- **DD** (Patch): 2-digit day with leading zero (01-31)
- **+commit**: Git commit hash (7 characters)

**Examples:**
- `2025.07.20+fc4e9d0` - Built on July 20th, 2025, from commit fc4e9d0
- `2025.12.31+a1b2c3d` - Built on December 31st, 2025, from commit a1b2c3d

### Version Components

| Component | CMake Variable | Range | Purpose |
|-----------|----------------|-------|---------|
| Major (Year) | `OPENAUTO_BUILD_YEAR` | 2025+ | Calendar year of build |
| Minor (Month) | `OPENAUTO_BUILD_MONTH` | 01-12 | Calendar month of build |
| Patch (Day) | `OPENAUTO_BUILD_DAY` | 01-31 | Calendar day of build |
| Build ID | `OPENAUTO_GIT_COMMIT_SHORT` | 7 chars | Git commit identifier |

### Git Integration

The build system automatically captures comprehensive git information:

```bash
# Git Information Captured:
- Branch name (e.g., crankshaft-ng_2025)
- Short commit hash (7 chars, e.g., fc4e9d0)
- Full commit hash (40 chars)
- Git describe output (includes tag info and dirty status)
- Working tree status (clean/dirty)
```

### CMake Implementation

Version detection happens automatically during CMake configuration:

```cmake
# Date-based version components
string(TIMESTAMP OPENAUTO_BUILD_YEAR "%Y")      # 2025
string(TIMESTAMP OPENAUTO_BUILD_MONTH "%m")     # 07  
string(TIMESTAMP OPENAUTO_BUILD_DAY "%d")       # 20

# Git information capture
execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
    OUTPUT_VARIABLE OPENAUTO_GIT_COMMIT_SHORT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Final version string: 2025.07.20+fc4e9d0
set(PROGRAM_VERSION_STRING "${OPENAUTO_BUILD_YEAR}.${OPENAUTO_BUILD_MONTH}.${OPENAUTO_BUILD_DAY}+${OPENAUTO_GIT_COMMIT_SHORT}")
```

### Runtime Version Access

The `Version.hpp` header provides C++ access to version information:

```cpp
#include "modern/Version.hpp"

// Basic version info
std::cout << "Version: " << openauto::modern::getVersionString() << std::endl;
// Output: 2025.07.20+fc4e9d0

// Detailed version info  
std::cout << "Build Info: " << openauto::modern::getBuildInfo() << std::endl;
// Output: Built on 20250720 from branch crankshaft-ng_2025 (fc4e9d0)

// Full version details
std::cout << openauto::modern::getFullVersionInfo() << std::endl;
```

### Version Validation Tools

Several scripts help verify version information:

#### `check-version.sh`
Comprehensive version validation and CMake integration test:

```bash
./check-version.sh
# Output:
# 🔍 OpenAuto Version Information
# ==================================
# 📅 Date-based Version Components:
#   Major (Year):  2025
#   Minor (Month): 07  
#   Patch (Day):   20
# 🔗 Git Information:
#   Branch:        crankshaft-ng_2025
#   Commit:        fc4e9d0
#   Working Tree:  CLEAN
# 🎯 Final Version: 2025.07.20+fc4e9d0
```

#### Version Display in Build Scripts
The `build-packages.sh` script shows version info during builds:

```bash
./build-packages.sh --release-only
# 📦 OpenAuto Package Builder
# Version: 2025.07.20+fc4e9d0
# Git: fc4e9d0 (crankshaft-ng_2025)
# Architecture: armhf
```

### Package Versioning

Debian packages use the complete version string:

```
Package: openauto-modern_2025.07.20+fc4e9d0-1_armhf.deb
Version: 2025.07.20+fc4e9d0
```

Debug packages have separate versioning:
```
Package: openauto-modern-debug_2025.07.20+fc4e9d0-1_armhf.deb  
```

## VS Code Development Environment

OpenAuto provides comprehensive VS Code integration for efficient development workflow.

### VS Code Tasks Configuration

The `.vscode/tasks.json` file contains **18 specialized tasks** for complete build/test/deploy automation:

### 🏗️ Build Tasks

#### Primary Build Tasks
| Task | Purpose | Default |
|------|---------|---------|
| **Build Release Package** | Build optimized production package | ✓ (Default Build) |
| **Build Debug Package** | Build debug package with symbols | |
| **Build Both Packages** | Build both release and debug packages | |

#### Low-Level Build Tasks
| Task | Purpose |
|------|---------|
| **Configure Release Build** | Run CMake configuration for release |
| **Configure Debug Build** | Run CMake configuration for debug |
| **Build Release (CMake)** | Direct CMake build (release) |
| **Build Debug (CMake)** | Direct CMake build (debug) |
| **Clean All Builds** | Remove all build artifacts |

### 🧪 Test Tasks

| Task | Purpose | Default |
|------|---------|---------|
| **Run Tests** | Execute test suite | ✓ (Default Test) |
| **Run Tests (Verbose)** | Execute tests with detailed output | |
| **Validate Packages** | Verify package integrity | |
| **Check Version Info** | Validate version system | |

### 📦 Deployment Tasks

| Task | Purpose |
|------|---------|
| **Install Release Package** | Install production package |
| **Install Debug Package** | Install debug package |
| **Start OpenAuto Service** | Start systemd service |
| **Check Service Status** | View service status |
| **View Service Logs** | Monitor service logs (background) |

### 🚀 Pipeline Task

| Task | Purpose |
|------|---------|
| **Full Build and Test Pipeline** | Complete CI/CD-style workflow |

### Task Features

#### Problem Matchers
C++ compilation errors are automatically parsed and displayed in VS Code's Problems panel:

```json
"problemMatcher": {
    "owner": "cpp",
    "fileLocation": "absolute",
    "pattern": {
        "regexp": "^(.*):(\\d+):(\\d+):\\s+(warning|error):\\s+(.*)$",
        "file": 1,
        "line": 2,
        "column": 3,
        "severity": 4,
        "message": 5
    }
}
```

#### Task Dependencies
Complex build pipelines with automatic dependency resolution:

```json
"dependsOn": [
    "Clean All Builds",
    "Build Both Packages", 
    "Validate Packages",
    "Check Version Info"
]
```

#### Input Variables
Configurable build parameters:

```json
"args": ["--parallel", "${input:numberOfJobs}"]

"inputs": [
    {
        "id": "numberOfJobs",
        "description": "Number of parallel jobs",
        "default": "4",
        "type": "promptString"
    }
]
```

### Using VS Code Tasks

#### Running Tasks

1. **Command Palette**: 
   - `Ctrl+Shift+P` → "Tasks: Run Task" → Select task
   
2. **Default Tasks**:
   - `Ctrl+Shift+P` → "Tasks: Run Build Task" (Build Release Package)
   - `Ctrl+Shift+P` → "Tasks: Run Test Task" (Run Tests)

3. **Keyboard Shortcuts**: Can be configured in VS Code settings

#### Task Output

All tasks provide:
- ✅ **Real-time output** in integrated terminal
- ✅ **Error highlighting** via problem matchers  
- ✅ **Progress indication** with status messages
- ✅ **Background execution** for long-running tasks

### Development Workflow Examples

#### Standard Development Cycle
```
1. Edit code
2. Run "Build Debug Package" task
3. Run "Run Tests" task  
4. Fix any issues shown in Problems panel
5. Run "Install Debug Package" for testing
```

#### Production Release Cycle
```
1. Run "Full Build and Test Pipeline" task
2. Validate all tests pass
3. Run "Build Release Package" task
4. Run "Validate Packages" task
5. Deploy package to production
```

#### Debugging Session
```
1. Run "Build Debug Package" task
2. Run "Install Debug Package" task
3. Run "Start OpenAuto Service" task
4. Run "View Service Logs" (background monitoring)
5. Debug with logs and symbols available
```

### Integration with Version System

VS Code tasks automatically display version information:

```bash
# Task Output Example:
📦 OpenAuto Package Builder  
==========================
🎯 Version: 2025.07.20+fc4e9d0
🌿 Branch: crankshaft-ng_2025  
📝 Commit: fc4e9d0
🏗️ Architecture: armhf
📅 Build Date: 20250720

✅ Release package built successfully!
📦 Package: openauto-modern_2025.07.20+fc4e9d0-1_armhf.deb
```

### Performance Features

- **Parallel Builds**: Configurable job count for faster compilation
- **Incremental Builds**: CMake handles dependency tracking  
- **Background Tasks**: Long-running processes don't block UI
- **Task Caching**: Dependencies only run when needed

### Troubleshooting VS Code Tasks

#### Common Issues

**Task Not Found:**
- Ensure `.vscode/tasks.json` exists in workspace root
- Reload VS Code window (`Ctrl+Shift+P` → "Reload Window")

**Build Errors:**
- Check Problems panel for compilation errors
- Verify dependencies are installed  
- Run "Clean All Builds" and retry

**Permission Issues:**
- Debug and release packages conflict - uninstall before switching
- Service tasks may require sudo permissions

**JSON Validation Errors:**
- VS Code validates `tasks.json` syntax automatically
- Check for missing commas, brackets, or invalid problem matcher references

### VS Code Extensions Integration

Recommended extensions for OpenAuto development:

- **C/C++** (Microsoft) - IntelliSense and debugging
- **CMake Tools** - CMake integration
- **GitLens** - Enhanced git information  
- **Error Lens** - Inline error display
- **Task Explorer** - Visual task management

## Version Scheme Benefits

### 1. **Chronological Clarity**
- Instantly know when any version was built
- Easy to identify newer vs older versions
- Natural sorting aligns with chronological order

### 2. **Git Integration**  
- Every build traceable to exact source code state
- Commit hash enables quick source checkout
- Branch information shows development context

### 3. **Automated Generation**
- No manual version bumping required
- Consistent across all build environments
- Eliminates human error in versioning

### 4. **Debugging Benefits**
- Link runtime issues to exact source code
- Reproduce builds from any point in history
- Track feature introduction via commit history

### 5. **Deployment Tracking**
- Production environments show exact build date/commit
- Easy correlation between deployments and source changes
- Simplified rollback to previous versions

## Migration from Traditional Versioning

For projects moving from semantic versioning (e.g., v1.2.3):

### Advantages of Date-Based Versioning

| Traditional (v1.2.3) | Date-Based (2025.07.20+commit) |
|-----------------------|--------------------------------|  
| Manual version bumps | Automatic generation |
| Subjective significance | Objective chronology |
| Git correlation unclear | Direct git integration |
| Deployment ambiguity | Clear build timestamp |
| Version conflicts possible | Unique by definition |

### Implementation Strategy

1. **Parallel Period**: Run both versioning schemes temporarily
2. **Tooling Update**: Update scripts, CI/CD, monitoring systems
3. **Documentation**: Update user-facing documentation  
4. **Gradual Rollout**: Deploy to development, staging, then production
5. **Legacy Cleanup**: Remove old versioning after validation

This date-based versioning system combined with comprehensive VS Code integration provides a modern, efficient development environment for OpenAuto that scales from individual development to enterprise deployment.
