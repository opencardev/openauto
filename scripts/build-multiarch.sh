#!/bin/bash
# Enhanced build script for multi-architecture package building

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Script configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Default values
BUILD_ARCHITECTURES=("amd64")
BUILD_TYPE="Release"
PACKAGE_OUTPUT_DIR="packages"
CLEAN_BUILD=true
DOCKER_BUILD=false
VALIDATE_PACKAGES=true
CREATE_DEBUG_PACKAGES=true

# Help function
show_help() {
    cat << EOF
OpenAuto Multi-Architecture Package Builder

Usage: $0 [OPTIONS]

OPTIONS:
    -a, --arch ARCH          Target architecture (amd64, arm64, armhf, all)
                             Can be specified multiple times
    -t, --type TYPE          Build type (Release, Debug) [default: Release]
    -o, --output DIR         Output directory for packages [default: packages]
    --no-clean              Don't clean build directories
    --docker                Use Docker for cross-compilation
    --no-validate           Skip package validation
    --no-debug              Don't create debug packages
    -h, --help              Show this help message

EXAMPLES:
    $0                                    # Build for amd64 only
    $0 -a all                            # Build for all architectures
    $0 -a arm64 -a armhf                 # Build for ARM64 and ARMHF
    $0 -a amd64 --docker                 # Build AMD64 using Docker
    $0 -t Debug --no-validate           # Build debug packages without validation

ARCHITECTURES:
    amd64     - x86_64 (Intel/AMD 64-bit)
    arm64     - AArch64 (ARM 64-bit)
    armhf     - ARM hard float (ARM 32-bit)
    all       - Build for all supported architectures

EOF
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -a|--arch)
            if [[ "$2" == "all" ]]; then
                BUILD_ARCHITECTURES=("amd64" "arm64" "armhf")
            else
                BUILD_ARCHITECTURES+=("$2")
            fi
            shift 2
            ;;
        -t|--type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -o|--output)
            PACKAGE_OUTPUT_DIR="$2"
            shift 2
            ;;
        --no-clean)
            CLEAN_BUILD=false
            shift
            ;;
        --docker)
            DOCKER_BUILD=true
            shift
            ;;
        --no-validate)
            VALIDATE_PACKAGES=false
            shift
            ;;
        --no-debug)
            CREATE_DEBUG_PACKAGES=false
            shift
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            show_help
            exit 1
            ;;
    esac
done

# Remove duplicates from architectures array
BUILD_ARCHITECTURES=($(printf "%s\n" "${BUILD_ARCHITECTURES[@]}" | sort -u))

echo -e "${BLUE}🏗️  OpenAuto Multi-Architecture Package Builder${NC}"
echo "=================================================="
echo -e "${YELLOW}Configuration:${NC}"
echo "  Architectures: ${BUILD_ARCHITECTURES[*]}"
echo "  Build Type: $BUILD_TYPE"
echo "  Output Directory: $PACKAGE_OUTPUT_DIR"
echo "  Docker Build: $DOCKER_BUILD"
echo "  Validate Packages: $VALIDATE_PACKAGES"
echo "  Create Debug Packages: $CREATE_DEBUG_PACKAGES"
echo ""

# Get version information
CURRENT_YEAR=$(date +%Y)
CURRENT_MONTH=$(date +%m)
CURRENT_DAY=$(date +%d)
GIT_COMMIT=$(git rev-parse --short HEAD 2>/dev/null || echo "unknown")
GIT_BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "unknown")
VERSION="${CURRENT_YEAR}.${CURRENT_MONTH}.${CURRENT_DAY}+${GIT_COMMIT}"
PACKAGE_VERSION="${CURRENT_YEAR}.${CURRENT_MONTH}.${CURRENT_DAY}~${GIT_COMMIT}"

echo -e "${YELLOW}📅 Version Information:${NC}"
echo "  Version: $VERSION"
echo "  Package Version: $PACKAGE_VERSION"
echo "  Branch: $GIT_BRANCH"
echo "  Date: $(date '+%Y-%m-%d %H:%M:%S')"
echo ""

# Clean previous builds if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo -e "${YELLOW}🧹 Cleaning previous builds...${NC}"
    rm -rf build build-* "$PACKAGE_OUTPUT_DIR"
    echo "✓ Cleaned"
fi

# Create output directory
mkdir -p "$PACKAGE_OUTPUT_DIR"

# Function to build for a specific architecture
build_architecture() {
    local arch="$1"
    local build_dir="build-${arch}"
    
    echo -e "${BLUE}🔨 Building for ${arch}...${NC}"
    
    case "$arch" in
        amd64)
            build_native "$arch" "$build_dir"
            ;;
        arm64|armhf)
            if [ "$DOCKER_BUILD" = true ]; then
                build_with_docker "$arch" "$build_dir"
            else
                build_cross_compile "$arch" "$build_dir"
            fi
            ;;
        *)
            echo -e "${RED}❌ Unsupported architecture: $arch${NC}"
            return 1
            ;;
    esac
    
    # Create packages
    create_packages "$arch" "$build_dir"
    
    echo -e "${GREEN}✓ Successfully built for ${arch}${NC}"
}

# Native build function (for amd64)
build_native() {
    local arch="$1"
    local build_dir="$2"
    
    echo "  Installing dependencies..."
    install_native_dependencies
    
    echo "  Building AASDK..."
    build_aasdk_native
    
    echo "  Configuring CMake..."
    cmake -B "$build_dir" -S . \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DCMAKE_CXX_STANDARD=17 \
        -DNOPI=ON \
        -DENABLE_MODERN_API=ON \
        -DENABLE_CS_PARAM_CALLS=OFF \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -GNinja
    
    echo "  Building..."
    cmake --build "$build_dir" --parallel $(nproc)
}

# Cross-compilation build function
build_cross_compile() {
    local arch="$1"
    local build_dir="$2"
    local target
    
    case "$arch" in
        arm64)
            target="aarch64-linux-gnu"
            ;;
        armhf)
            target="arm-linux-gnueabihf"
            ;;
    esac
    
    echo "  Installing cross-compilation tools..."
    install_cross_dependencies "$arch" "$target"
    
    echo "  Building AASDK for $arch..."
    build_aasdk_cross "$arch" "$target"
    
    echo "  Configuring CMake for cross-compilation..."
    cmake -B "$build_dir" -S . \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DCMAKE_CXX_STANDARD=17 \
        -DNOPI=ON \
        -DENABLE_MODERN_API=ON \
        -DENABLE_CS_PARAM_CALLS=OFF \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DCMAKE_SYSTEM_NAME=Linux \
        -DCMAKE_SYSTEM_PROCESSOR="${arch}" \
        -DCMAKE_C_COMPILER="${target}-gcc" \
        -DCMAKE_CXX_COMPILER="${target}-g++" \
        -GNinja
    
    echo "  Building..."
    cmake --build "$build_dir" --parallel $(nproc)
}

# Docker build function
build_with_docker() {
    local arch="$1"
    local build_dir="$2"
    
    echo "  Building with Docker for $arch..."
    
    # Create Dockerfile for cross-compilation
    create_docker_cross_file "$arch"
    
    # Build Docker image
    docker build -f "Dockerfile.cross-${arch}" -t "openauto-cross-${arch}" .
    
    # Run build in container
    local platform
    case "$arch" in
        arm64) platform="linux/arm64" ;;
        armhf) platform="linux/arm/v7" ;;
    esac
    
    docker run --rm --platform="$platform" \
        -v "$(pwd):/workspace" \
        "openauto-cross-${arch}" \
        bash -c "cd /workspace && ./scripts/build-in-docker.sh $arch $build_dir $BUILD_TYPE"
}

# Package creation function
create_packages() {
    local arch="$1"
    local build_dir="$2"
    local package_dir="$PACKAGE_OUTPUT_DIR/$arch"
    
    echo "  Creating packages for $arch..."
    
    # Create package directory structure
    mkdir -p "$package_dir/install"
    DESTDIR="$(pwd)/$package_dir/install" cmake --install "$build_dir"
    
    # Create main package
    create_main_package "$arch" "$package_dir"
    
    # Create debug package if requested
    if [ "$CREATE_DEBUG_PACKAGES" = true ]; then
        create_debug_package "$arch" "$package_dir"
    fi
}

# Main package creation
create_main_package() {
    local arch="$1"
    local package_dir="$2"
    
    mkdir -p "$package_dir/DEBIAN"
    
    # Create control file
    cat > "$package_dir/DEBIAN/control" << EOF
Package: openauto-modern
Version: $PACKAGE_VERSION
Section: multimedia
Priority: optional
Architecture: $arch
Depends: libboost-all-dev, libprotobuf32, qtbase5-dev, qtmultimedia5-dev, qtdeclarative5-dev, qtquickcontrols2-5-dev, libusb-1.0-0, libtag1v5, librtaudio6, libasound2, libpulse0
Maintainer: OpenCarDev Team <team@opencardev.com>
Description: Modern Android Auto implementation
 OpenAuto is a modern implementation of Android Auto head unit emulator.
 .
 This package provides the main application and all required components
 to run Android Auto on various hardware platforms.
Homepage: https://github.com/opencardev/openauto
EOF

    # Copy packaging scripts
    if [ -d "packaging/debian" ]; then
        cp -r packaging/debian/* "$package_dir/DEBIAN/" 2>/dev/null || true
    fi
    
    # Copy system files
    copy_system_files "$package_dir"
    
    # Build package
    cd "$PACKAGE_OUTPUT_DIR"
    fakeroot dpkg-deb --build "$arch" "openauto-modern_${PACKAGE_VERSION}_${arch}.deb"
    cd - > /dev/null
}

# Debug package creation
create_debug_package() {
    local arch="$1"
    local package_dir="$2"
    local debug_dir="${package_dir}-debug"
    
    mkdir -p "$debug_dir/DEBIAN"
    mkdir -p "$debug_dir/install/usr/lib/debug"
    
    # Extract debug symbols
    find "$package_dir/install" -type f -executable | while read file; do
        if file "$file" | grep -q "not stripped"; then
            debug_path="$debug_dir/install/usr/lib/debug${file#$package_dir/install}"
            mkdir -p "$(dirname "$debug_path")"
            objcopy --only-keep-debug "$file" "$debug_path"
            objcopy --strip-debug "$file"
            objcopy --add-gnu-debuglink="$debug_path" "$file"
        fi
    done
    
    # Create debug control file
    cat > "$debug_dir/DEBIAN/control" << EOF
Package: openauto-modern-dbg
Version: $PACKAGE_VERSION
Section: debug
Priority: optional
Architecture: $arch
Depends: openauto-modern (= $PACKAGE_VERSION)
Maintainer: OpenCarDev Team <team@opencardev.com>
Description: Debug symbols for OpenAuto Modern
 This package contains debug symbols for OpenAuto Modern.
EOF

    # Build debug package
    cd "$PACKAGE_OUTPUT_DIR"
    fakeroot dpkg-deb --build "${arch}-debug" "openauto-modern-dbg_${PACKAGE_VERSION}_${arch}.deb"
    cd - > /dev/null
}

# System files copying
copy_system_files() {
    local package_dir="$1"
    
    # systemd service files
    if [ -d "packaging/systemd" ]; then
        mkdir -p "$package_dir/install/lib/systemd/system"
        cp -r packaging/systemd/* "$package_dir/install/lib/systemd/system/" 2>/dev/null || true
    fi
    
    # udev rules
    if [ -d "packaging/udev" ]; then
        mkdir -p "$package_dir/install/lib/udev/rules.d"
        cp -r packaging/udev/* "$package_dir/install/lib/udev/rules.d/" 2>/dev/null || true
    fi
    
    # configuration files
    if [ -d "packaging/config" ]; then
        mkdir -p "$package_dir/install/etc/openauto"
        cp -r packaging/config/* "$package_dir/install/etc/openauto/" 2>/dev/null || true
    fi
}

# Dependency installation functions
install_native_dependencies() {
    if command -v apt-get >/dev/null 2>&1; then
        sudo apt-get update
        sudo apt-get install -y \
            build-essential \
            cmake \
            ninja-build \
            pkg-config \
            libboost-all-dev \
            libprotobuf-dev \
            protobuf-compiler \
            nlohmann-json3-dev \
            qtbase5-dev \
            qtmultimedia5-dev \
            qtdeclarative5-dev \
            qtquickcontrols2-5-dev \
            libusb-1.0-0-dev \
            libtag1-dev \
            librtaudio-dev \
            libasound2-dev \
            libpulse-dev \
            debhelper \
            fakeroot \
            devscripts
    fi
}

install_cross_dependencies() {
    local arch="$1"
    local target="$2"
    
    if command -v apt-get >/dev/null 2>&1; then
        sudo apt-get update
        sudo apt-get install -y \
            "gcc-${target}" \
            "g++-${target}" \
            "pkg-config-${target}"
        
        # Add target architecture and install cross dependencies
        local debian_arch
        case "$arch" in
            arm64) debian_arch="arm64" ;;
            armhf) debian_arch="armhf" ;;
        esac
        
        sudo dpkg --add-architecture "$debian_arch"
        sudo apt-get update
        sudo apt-get install -y \
            "libboost-all-dev:${debian_arch}" \
            "libprotobuf-dev:${debian_arch}" \
            "qtbase5-dev:${debian_arch}" \
            "qtmultimedia5-dev:${debian_arch}" \
            "qtdeclarative5-dev:${debian_arch}" \
            "qtquickcontrols2-5-dev:${debian_arch}" \
            "libusb-1.0-0-dev:${debian_arch}" \
            "libtag1-dev:${debian_arch}" \
            "librtaudio-dev:${debian_arch}" \
            "libasound2-dev:${debian_arch}" \
            "libpulse-dev:${debian_arch}"
    fi
}

# AASDK build functions
build_aasdk_native() {
    if [ ! -d "/tmp/aasdk" ]; then
        git clone https://github.com/opencardev/aasdk.git /tmp/aasdk
    fi
    
    cd /tmp/aasdk
    mkdir -p build && cd build
    cmake .. \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DCMAKE_CXX_STANDARD=17 \
        -DCMAKE_INSTALL_PREFIX=/usr/local \
        -DTARGET_ARCH=amd64 \
        -GNinja
    ninja -j$(nproc)
    sudo ninja install
    sudo ldconfig
    cd "$PROJECT_ROOT"
}

build_aasdk_cross() {
    local arch="$1"
    local target="$2"
    
    if [ ! -d "/tmp/aasdk-${arch}" ]; then
        git clone https://github.com/opencardev/aasdk.git "/tmp/aasdk-${arch}"
    fi
    
    cd "/tmp/aasdk-${arch}"
    mkdir -p "build-${arch}" && cd "build-${arch}"
    cmake .. \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DCMAKE_CXX_STANDARD=17 \
        -DCMAKE_INSTALL_PREFIX=/usr/local \
        -DCMAKE_SYSTEM_NAME=Linux \
        -DCMAKE_SYSTEM_PROCESSOR="$arch" \
        -DCMAKE_C_COMPILER="${target}-gcc" \
        -DCMAKE_CXX_COMPILER="${target}-g++" \
        -DTARGET_ARCH="$arch" \
        -GNinja
    ninja -j$(nproc)
    sudo ninja install
    sudo ldconfig
    cd "$PROJECT_ROOT"
}

# Docker file creation
create_docker_cross_file() {
    local arch="$1"
    local debian_arch
    local target
    
    case "$arch" in
        arm64)
            debian_arch="arm64"
            target="aarch64-linux-gnu"
            ;;
        armhf)
            debian_arch="armhf"
            target="arm-linux-gnueabihf"
            ;;
    esac
    
    cat > "Dockerfile.cross-${arch}" << EOF
FROM debian:bookworm

RUN apt-get update && apt-get install -y \\
    build-essential \\
    cmake \\
    ninja-build \\
    pkg-config \\
    git \\
    wget \\
    curl \\
    debhelper \\
    fakeroot \\
    devscripts \\
    gcc-${target} \\
    g++-${target} \\
    pkg-config-${target}

RUN dpkg --add-architecture ${debian_arch} && \\
    apt-get update && \\
    apt-get install -y \\
    libboost-all-dev:${debian_arch} \\
    libprotobuf-dev:${debian_arch} \\
    protobuf-compiler \\
    nlohmann-json3-dev \\
    qtbase5-dev:${debian_arch} \\
    qtmultimedia5-dev:${debian_arch} \\
    qtdeclarative5-dev:${debian_arch} \\
    qtquickcontrols2-5-dev:${debian_arch} \\
    libusb-1.0-0-dev:${debian_arch} \\
    libtag1-dev:${debian_arch} \\
    librtaudio-dev:${debian_arch} \\
    libasound2-dev:${debian_arch} \\
    libpulse-dev:${debian_arch}

WORKDIR /workspace
EOF
}

# Package validation function
validate_all_packages() {
    if [ "$VALIDATE_PACKAGES" = false ]; then
        return 0
    fi
    
    echo -e "${BLUE}🔍 Validating packages...${NC}"
    
    cd "$PACKAGE_OUTPUT_DIR"
    local validation_failed=false
    
    for deb in *.deb; do
        if [ -f "$deb" ]; then
            echo "  Validating $deb..."
            
            # Basic package validation
            if ! dpkg-deb --info "$deb" >/dev/null 2>&1; then
                echo -e "    ${RED}❌ Invalid package format${NC}"
                validation_failed=true
                continue
            fi
            
            # Check package contents
            if ! dpkg-deb --contents "$deb" | grep -q "usr/bin/autoapp"; then
                echo -e "    ${RED}❌ Missing main binary${NC}"
                validation_failed=true
            else
                echo -e "    ${GREEN}✓ Main binary found${NC}"
            fi
            
            # Run lintian if available
            if command -v lintian >/dev/null 2>&1; then
                if lintian --quiet "$deb" 2>/dev/null; then
                    echo -e "    ${GREEN}✓ Lintian checks passed${NC}"
                else
                    echo -e "    ${YELLOW}⚠ Lintian warnings found${NC}"
                fi
            fi
        fi
    done
    
    cd - > /dev/null
    
    if [ "$validation_failed" = true ]; then
        echo -e "${RED}❌ Package validation failed${NC}"
        return 1
    else
        echo -e "${GREEN}✓ All packages validated successfully${NC}"
    fi
}

# Main build loop
echo -e "${BLUE}🚀 Starting build process...${NC}"

for arch in "${BUILD_ARCHITECTURES[@]}"; do
    echo ""
    build_architecture "$arch"
done

echo ""
echo -e "${BLUE}📦 Package Summary:${NC}"
cd "$PACKAGE_OUTPUT_DIR"
for deb in *.deb; do
    if [ -f "$deb" ]; then
        size=$(du -h "$deb" | cut -f1)
        echo "  $deb ($size)"
    fi
done
cd - > /dev/null

# Validate packages
echo ""
validate_all_packages

echo ""
echo -e "${GREEN}🎉 Build completed successfully!${NC}"
echo -e "${YELLOW}📁 Packages are available in: $PACKAGE_OUTPUT_DIR${NC}"
echo ""
