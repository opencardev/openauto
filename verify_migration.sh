#!/bin/bash
#
# OpenAuto Modern Logger Build Verification Script
# 
# This script helps identify and fix common build issues when migrating
# to the modern logger system.
#

echo "=== OpenAuto Modern Logger Build Verification ==="
echo

# Function to check for legacy files
check_legacy_files() {
    echo "🔍 Checking for legacy EventBus files..."
    
    LEGACY_EVENTBUS=$(find . -path "./build" -prune -o -name "*EventBus*" -print 2>/dev/null)
    if [ -n "$LEGACY_EVENTBUS" ]; then
        echo "⚠️  Legacy EventBus files found:"
        echo "$LEGACY_EVENTBUS"
        echo
        echo "These files can cause build conflicts. Consider removing them:"
        echo "rm -rf $LEGACY_EVENTBUS"
        echo
        return 1
    else
        echo "✅ No legacy EventBus files found"
    fi
    return 0
}

# Function to check for legacy includes
check_legacy_includes() {
    echo "🔍 Checking for legacy include statements..."
    
    LEGACY_INCLUDES=$(grep -r "f1x.*EventBus" . --exclude-dir=build --exclude-dir=.git 2>/dev/null || true)
    if [ -n "$LEGACY_INCLUDES" ]; then
        echo "⚠️  Legacy include statements found:"
        echo "$LEGACY_INCLUDES"
        echo
        echo "These includes should be updated to use modern includes:"
        echo '#include "modern/Event.hpp"'
        echo '#include "modern/EventBus.hpp"'
        echo
        return 1
    else
        echo "✅ No legacy include statements found"
    fi
    return 0
}

# Function to check for legacy log includes
check_legacy_log_includes() {
    echo "🔍 Checking for legacy logging includes..."
    
    LEGACY_LOG_INCLUDES=$(grep -r "f1x/openauto/Common/Log.hpp" . --exclude-dir=build --exclude-dir=.git 2>/dev/null || true)
    if [ -n "$LEGACY_LOG_INCLUDES" ]; then
        echo "⚠️  Legacy logging includes found:"
        echo "$LEGACY_LOG_INCLUDES" | head -10
        if [ $(echo "$LEGACY_LOG_INCLUDES" | wc -l) -gt 10 ]; then
            echo "... and $(echo "$LEGACY_LOG_INCLUDES" | wc -l | awk '{print $1-10}') more"
        fi
        echo
        echo "These should be updated to:"
        echo '#include "modern/Logger.hpp"'
        echo
        return 1
    else
        echo "✅ No legacy logging includes found"
    fi
    return 0
}

# Function to check build directory
check_build_directory() {
    echo "🔍 Checking build directory status..."
    
    if [ -d "build" ]; then
        echo "⚠️  Build directory exists"
        echo "For a clean migration, consider removing it:"
        echo "rm -rf build/"
        echo
        return 1
    else
        echo "✅ No build directory found (clean state)"
    fi
    return 0
}

# Function to check modern files
check_modern_files() {
    echo "🔍 Checking for modern implementation files..."
    
    MODERN_FILES=(
        "include/modern/Logger.hpp"
        "include/modern/Event.hpp"
        "include/modern/EventBus.hpp"
        "src/modern/Logger.cpp"
        "src/modern/Event.cpp"
        "src/modern/EventBus.cpp"
    )
    
    MISSING_FILES=()
    for file in "${MODERN_FILES[@]}"; do
        if [ ! -f "$file" ]; then
            MISSING_FILES+=("$file")
        fi
    done
    
    if [ ${#MISSING_FILES[@]} -eq 0 ]; then
        echo "✅ All modern implementation files found"
        return 0
    else
        echo "⚠️  Missing modern implementation files:"
        for file in "${MISSING_FILES[@]}"; do
            echo "  - $file"
        done
        echo
        echo "Please ensure the modern architecture files are properly created."
        return 1
    fi
}

# Function to provide build recommendations
provide_recommendations() {
    echo "📋 Build Recommendations:"
    echo
    echo "1. Clean build:"
    echo "   rm -rf build/"
    echo "   mkdir build && cd build"
    echo
    echo "2. Configure with modern features:"
    echo "   cmake -DENABLE_MODERN_API=ON .."
    echo
    echo "3. Build with verbose output to catch issues:"
    echo "   make VERBOSE=1"
    echo
    echo "4. Test modern logger:"
    echo "   ./logger_demo  # If built with ENABLE_LOGGER_DEMO=ON"
    echo
}

# Main verification
main() {
    local issues=0
    
    # Run all checks
    check_legacy_files || ((issues++))
    echo
    
    check_legacy_includes || ((issues++))
    echo
    
    check_legacy_log_includes || ((issues++))
    echo
    
    check_build_directory || ((issues++))
    echo
    
    check_modern_files || ((issues++))
    echo
    
    # Summary
    if [ $issues -eq 0 ]; then
        echo "🎉 All checks passed! Your project is ready for modern logger build."
    else
        echo "⚠️  Found $issues potential issue(s). Please review the warnings above."
    fi
    
    echo
    provide_recommendations
}

# Run main function
main "$@"
