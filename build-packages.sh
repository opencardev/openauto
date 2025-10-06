#!/bin/bash
# Build script for OpenAuto packages (Release and Debug)

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BUILD_DIR="build"
RELEASE_BUILD_DIR="${BUILD_DIR}-release"
DEBUG_BUILD_DIR="${BUILD_DIR}-debug"
PACKAGE_OUTPUT_DIR="packages"

echo -e "${BLUE}🏗️  OpenAuto Package Builder${NC}"
echo "=================================="
echo "Date-based versioning: YYYY.MM.DD+commit"
echo ""

# Get current version info
CURRENT_YEAR=$(date +%Y)
CURRENT_MONTH=$(date +%m)
CURRENT_DAY=$(date +%d)
GIT_COMMIT=$(git rev-parse --short HEAD 2>/dev/null || echo "unknown")
GIT_BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "unknown")

echo -e "${YELLOW}📅 Build Information:${NC}"
echo "  Version: ${CURRENT_YEAR}.${CURRENT_MONTH}.${CURRENT_DAY}+${GIT_COMMIT}"
echo "  Branch: ${GIT_BRANCH}"
echo "  Date: $(date '+%Y-%m-%d %H:%M:%S')"
echo ""

# Parse command line arguments
BUILD_RELEASE=true
BUILD_DEBUG=true
CLEAN=true
NOPI=true  # Default to non-Pi build (NOPI=true)

while [[ $# -gt 0 ]]; do
    case $1 in
        --release-only)
            BUILD_DEBUG=false
            shift
            ;;
        --debug-only)
            BUILD_RELEASE=false
            shift
            ;;
        --no-clean)
            CLEAN=false
            shift
            ;;
        --pi)
            NOPI=false
            shift
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  --release-only  Build only release package"
            echo "  --debug-only    Build only debug package"
            echo "  --no-clean      Don't clean build directories"
            echo "  --pi            Build for Raspberry Pi 3 and lower"
            echo "  --help          Show this help"
            echo ""
            echo "Default behavior: Build for non-Pi systems (x86/x64)"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Display target platform
if [ "$NOPI" = true ]; then
    echo -e "${YELLOW}🎯 Target Platform: Non-Pi systems (x86/x64)${NC}"
else
    echo -e "${YELLOW}🎯 Target Platform: Raspberry Pi 3 and lower${NC}"
fi
echo ""

# Function to clean build directory
clean_build() {
    local build_dir=$1
    local build_type=$2
    
    echo -e "${YELLOW}🧹 Cleaning ${build_type} build directory...${NC}"
    rm -rf "$build_dir"
    mkdir -p "$build_dir"
}

# Function to check available memory and warn if low
check_memory() {
    local available_mem_mb=$(free -m | awk '/^Mem:/{print $7}')
    if [ "$available_mem_mb" -lt 1000 ]; then
        echo -e "${RED}⚠️  Critical: Very low memory (${available_mem_mb}MB available)${NC}"
        echo -e "${YELLOW}💡 Consider:${NC}"
        echo -e "${YELLOW}   1. Closing other applications${NC}"
        echo -e "${YELLOW}   2. Increasing swap space${NC}"
        echo -e "${YELLOW}   3. Building on a machine with more RAM${NC}"
        read -p "Continue anyway? (y/N): " -r
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            echo -e "${RED}❌ Build cancelled by user${NC}"
            exit 1
        fi
    elif [ "$available_mem_mb" -lt 2000 ]; then
        echo -e "${YELLOW}⚠️  Low memory detected (${available_mem_mb}MB available)${NC}"
        echo -e "${YELLOW}💡 Will use single-threaded compilation with memory optimization${NC}"
    fi
}

# Function to create a memory-optimized make wrapper
memory_optimized_make() {
    local jobs=$1
    local build_type=$2
    
    if [ "$jobs" -eq 1 ] && [ "$(free -m | awk '/^Mem:/{print $7}')" -lt 2000 ]; then
        echo -e "${YELLOW}⚙️  Using ultra-conservative memory build mode...${NC}"
        
        # Get list of source files to compile them one by one if needed
        local cpp_files=$(find ../src -name "*.cpp" | wc -l)
        echo -e "${YELLOW}⚙️  Found ${cpp_files} C++ source files to compile${NC}"
        
        # Try normal make first, but with strict memory monitoring
        if ! timeout 1800 make -j1 VERBOSE=0; then
            echo -e "${RED}⚠️  Normal build failed, trying ultra-conservative approach...${NC}"
            
            # If that fails, try building with even more conservative settings
            export CXXFLAGS="${CXXFLAGS} -Os -fno-inline-functions -fno-omit-frame-pointer"
            echo -e "${YELLOW}⚙️  Retrying with size optimization and reduced inlining...${NC}"
            make clean
            timeout 2400 make -j1 VERBOSE=0
        fi
    else
        make -j${jobs}
    fi
}

# Function to configure and build
build_package() {
    local build_dir=$1
    local build_type=$2
    local cmake_flags=$3
    
    echo -e "${BLUE}📦 Building ${build_type} package...${NC}"
    
    # Check memory before starting
    check_memory
    
    cd "$build_dir"
    
    # Configure with CMake (add NOPI flag)
    local nopi_flag=""
    if [ "$NOPI" = true ]; then
        nopi_flag="-DNOPI=ON"
        echo -e "${YELLOW}⚙️  Configuring CMake for ${build_type} (Non-Pi build)...${NC}"
    else
        nopi_flag="-DNOPI=OFF"
        echo -e "${YELLOW}⚙️  Configuring CMake for ${build_type} (Raspberry Pi 3 and lower)...${NC}"
    fi
    
    cmake -DCMAKE_BUILD_TYPE="$build_type" $nopi_flag $cmake_flags ..
    
    # Build
    echo -e "${YELLOW}🔨 Compiling ${build_type} build...${NC}"
    # Use very conservative parallelism to avoid OOM issues on systems with limited RAM
    local available_mem_mb=$(free -m | awk '/^Mem:/{print $7}')
    local jobs=1
    
    # More conservative memory-based job calculation
    if [ "$available_mem_mb" -gt 6000 ]; then
        jobs=$(nproc)
    elif [ "$available_mem_mb" -gt 4000 ]; then
        jobs=2
    elif [ "$available_mem_mb" -gt 2000 ]; then
        jobs=1
    else
        jobs=1
        echo -e "${RED}⚠️  Warning: Very low memory detected (${available_mem_mb}MB available)${NC}"
        echo -e "${YELLOW}⚙️  Consider closing other applications or increasing swap space${NC}"
    fi
    
    echo -e "${YELLOW}⚙️  Using ${jobs} parallel job(s) (available memory: $(free -h | awk '/^Mem:/{print $7}'))${NC}"
    
    # Add memory monitoring during build
    if [ "$jobs" -eq 1 ]; then
        echo -e "${YELLOW}⚙️  Building with single-threaded compilation to conserve memory...${NC}"
        # For very memory-constrained builds, we use aggressive memory optimization
        export CXXFLAGS="${CXXFLAGS} -pipe -fno-debug-info-for-profiling"
        export CPPFLAGS="${CPPFLAGS} -DNDEBUG"
        
        # Use make with memory-conserving options
        echo -e "${YELLOW}⚙️  Using memory-conserving compilation flags...${NC}"
        # Use the memory-optimized make function
        memory_optimized_make ${jobs} ${build_type}
    else
        make -j${jobs}
    fi
    
    # Create package
    echo -e "${YELLOW}📦 Creating ${build_type} package...${NC}"
    make package
    
    cd ..
    
    # Move package to output directory
    mkdir -p "$PACKAGE_OUTPUT_DIR"
    mv "$build_dir"/*.deb "$PACKAGE_OUTPUT_DIR/" 2>/dev/null || true
    
    echo -e "${GREEN}✅ ${build_type} package completed${NC}"
}

# Create package output directory
mkdir -p "$PACKAGE_OUTPUT_DIR"

# Build Release Package
if [ "$BUILD_RELEASE" = true ]; then
    echo -e "${BLUE}🚀 Building Release Package${NC}"
    
    if [ "$CLEAN" = true ]; then
        clean_build "$RELEASE_BUILD_DIR" "Release"
    fi
    
    build_package "$RELEASE_BUILD_DIR" "Release" ""
fi

# Build Debug Package
if [ "$BUILD_DEBUG" = true ]; then
    echo -e "${BLUE}🐛 Building Debug Package${NC}"
    
    if [ "$CLEAN" = true ]; then
        clean_build "$DEBUG_BUILD_DIR" "Debug"
    fi
    
    build_package "$DEBUG_BUILD_DIR" "Debug" "-DENABLE_SANITIZERS=ON"
fi

echo ""
echo -e "${GREEN}🎉 Package Build Complete!${NC}"
echo "=================================="

# List created packages
if [ -d "$PACKAGE_OUTPUT_DIR" ] && [ "$(ls -A $PACKAGE_OUTPUT_DIR 2>/dev/null)" ]; then
    echo -e "${BLUE}📦 Created Packages:${NC}"
    ls -la "$PACKAGE_OUTPUT_DIR"/*.deb 2>/dev/null || true
    
    echo ""
    echo -e "${YELLOW}📋 Installation Commands:${NC}"
    for deb in "$PACKAGE_OUTPUT_DIR"/*.deb; do
        if [ -f "$deb" ]; then
            echo "  sudo apt install $(realpath "$deb")"
        fi
    done
    
    echo ""
    echo -e "${YELLOW}🔍 Package Information:${NC}"
    for deb in "$PACKAGE_OUTPUT_DIR"/*.deb; do
        if [ -f "$deb" ]; then
            echo -e "${BLUE}$(basename "$deb"):${NC}"
            dpkg-deb --info "$deb" | grep -E "Package|Version|Architecture|Description" | sed 's/^/  /'
            echo ""
        fi
    done
else
    echo -e "${RED}❌ No packages were created${NC}"
    exit 1
fi
