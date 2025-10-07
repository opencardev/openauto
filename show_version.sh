#!/bin/bash
# OpenAuto Version Information Script
# This script shows the version information that will be compiled into the binary

echo "🚗 OpenAuto Version Information"
echo "=================================="

cd "$(dirname "$0")"

# Get git information
if command -v git >/dev/null 2>&1 && git rev-parse --git-dir >/dev/null 2>&1; then
    GIT_COMMIT_SHORT=$(git rev-parse --short HEAD 2>/dev/null || echo "unknown")
    GIT_COMMIT_FULL=$(git rev-parse HEAD 2>/dev/null || echo "unknown")
    GIT_BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "unknown")
    GIT_DESCRIBE=$(git describe --tags --dirty --always 2>/dev/null || echo "unknown")
else
    GIT_COMMIT_SHORT="unknown"
    GIT_COMMIT_FULL="unknown"  
    GIT_BRANCH="unknown"
    GIT_DESCRIBE="unknown"
fi

# Get date-based version components
BUILD_YEAR=$(date "+%Y")
BUILD_MONTH=$(date "+%m")
BUILD_DAY=$(date "+%d")
BUILD_DATE=$(date "+%Y%m%d")

# Create version string
VERSION_STRING="${BUILD_YEAR}.${BUILD_MONTH}.${BUILD_DAY}+${GIT_COMMIT_SHORT}"

echo "📅 Build Information:"
echo "   Date: $(date)"
echo "   Build Date: ${BUILD_DATE}"
echo ""
echo "🏷️  Version Information:"
echo "   Version String: ${VERSION_STRING}"
echo "   Major: ${BUILD_YEAR}"
echo "   Minor: ${BUILD_MONTH}" 
echo "   Patch: ${BUILD_DAY}"
echo ""
echo "🌿 Git Information:"
echo "   Branch: ${GIT_BRANCH}"
echo "   Commit (Short): ${GIT_COMMIT_SHORT}"
echo "   Commit (Full): ${GIT_COMMIT_FULL}"
echo "   Describe: ${GIT_DESCRIBE}"
echo ""
echo "🔧 C++ Preprocessor Definitions:"
echo "   -DOPENAUTO_VERSION_MAJOR=${BUILD_YEAR}"
echo "   -DOPENAUTO_VERSION_MINOR=${BUILD_MONTH}"
echo "   -DOPENAUTO_VERSION_PATCH=${BUILD_DAY}"
echo "   -DOPENAUTO_VERSION_STRING=\"${VERSION_STRING}\""
echo "   -DOPENAUTO_GIT_COMMIT=\"${GIT_COMMIT_SHORT}\""
echo "   -DOPENAUTO_GIT_COMMIT_FULL=\"${GIT_COMMIT_FULL}\""
echo "   -DOPENAUTO_GIT_BRANCH=\"${GIT_BRANCH}\""
echo "   -DOPENAUTO_GIT_DESCRIBE=\"${GIT_DESCRIBE}\""
echo "   -DOPENAUTO_BUILD_DATE=\"${BUILD_DATE}\""
echo ""
echo "📦 To build with this version:"
echo "   cmake -S . -B build && cmake --build build"
echo ""