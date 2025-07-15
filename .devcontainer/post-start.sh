#!/bin/bash

# Post-start script for OpenAuto development container
# This script runs every time the container starts

set -e

echo "🔄 Running post-start setup..."

# Ensure workspace permissions are correct
sudo chown -R vscode:vscode /workspace 2>/dev/null || true

# Start ccache daemon if not running
if command -v ccache >/dev/null 2>&1; then
    ccache -s >/dev/null 2>&1 || true
fi

# Check if build directory exists and has been configured
if [ ! -f /workspace/build/CMakeCache.txt ]; then
    echo "⚠️  CMake not configured. Run 'Configure CMake' task to set up the build."
fi

# Update compile commands for better IntelliSense
if [ -f /workspace/build/compile_commands.json ]; then
    ln -sf build/compile_commands.json /workspace/compile_commands.json 2>/dev/null || true
fi

# Display useful information
echo ""
echo "📋 Development Environment Status:"
echo "  • Workspace: /workspace"
echo "  • Build type: $(grep CMAKE_BUILD_TYPE /workspace/build/CMakeCache.txt 2>/dev/null | cut -d= -f2 || echo "Not configured")"
echo "  • Tests enabled: $(grep BUILD_TESTS /workspace/build/CMakeCache.txt 2>/dev/null | cut -d= -f2 || echo "Unknown")"
echo "  • ccache: $(ccache -s 2>/dev/null | grep "cache hit rate" || echo "Not available")"
echo ""
echo "🚀 Ready for development!"
echo ""
echo "Quick commands:"
echo "  • build  - Build the project"
echo "  • test   - Run all tests"
echo "  • run    - Start OpenAuto"
echo "  • clean  - Clean and reconfigure"
echo ""
