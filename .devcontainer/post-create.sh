#!/bin/bash

set -e

echo "🚀 Setting up OpenAuto development environment..."

# Set up workspace permissions
echo
sudo chown -R vscode:vscode /workspaces/openauto

# Make build scripts executable
echo "🔧 Making build scripts executable..."
chmod +x /workspaces/openauto/*.sh

# Create build directories
echo "📂 Creating build directories..."
mkdir -p /workspaces/openauto/build
mkdir -p /workspaces/openauto/build-release
mkdir -p /workspaces/openauto/build-debug
mkdir -p /workspaces/openauto/packages

# Set up ccache
echo "🔧 Setting up ccache..."
sudo mkdir -p /tmp/ccache
sudo chown vscode:vscode /tmp/ccache

# Initialize git if needed
echo "🔍 Initializing git repository if not already done..."
cd /workspaces/openauto
if [ ! -d .git ]; then
    git init
    git config --global user.name "OpenAuto Developer"  
    git config --global user.email "developer@openauto.dev"
fi

# Verify aasdk packages are installed
echo "🔍 Verifying aasdk installation..."
if dpkg -l | grep -q aasdk; then
    echo "✅ aasdk packages are installed:"
    dpkg -l | grep aasdk
else
    echo "⚠️  aasdk packages not found. Please check packages/ directory."
fi

# Check Qt5 installation
echo "🔍 Verifying Qt5 installation..."
if pkg-config --exists Qt5Core; then
    echo "✅ Qt5 is installed: $(pkg-config --modversion Qt5Core)"
else
    echo "⚠️  Qt5 not found or not properly configured."
fi

# install openauto scripts and udev
echo "🔍 Setting up OpenAuto scripts..."
sudo mkdir -p /opt/openauto/
sudo cp -r packaging/scripts /opt/openauto/
sudo chmod +x /opt/openauto/scripts/*.sh
# Check if udev is installed
echo "🔍 Checking udev installation..."
if ! command -v udevadm &> /dev/null; then
    echo "⚠️  udevadm not found. Please install udev package."
else
    echo "✅ udev is installed."
fi
# Copy udev rules
echo "🔍 Setting up udev rules..."
sudo cp -r packaging/udev/* /etc/udev/rules.d/
# Reload udev rules
sudo udevadm control --reload 2>/dev/null || true
sudo udevadm control --reload-rules 2>/dev/null || true
sudo udevadm trigger 2>/dev/null || true
# Set up environment variables
echo "export OPENAUTO_ROOT=/workspaces/openauto" >> /home/vscode/.bashrc
echo "export PATH=\$PATH:/opt/openauto/scripts" >> /home/vscode/.bashrc
echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:/opt/openauto/lib" >> /home/vscode/.bashrc
# Source the updated .bashrc
source /home/vscode/.bashrc

# Display version information
echo ""
echo "📦 OpenAuto Development Environment Ready!"
echo "========================================="
echo "Current directory: $(pwd)"
echo "Container architecture: $(uname -m)"
echo "Available build scripts:"
ls -la *.sh 2>/dev/null | grep -E '\.(sh)$' || echo "  (No build scripts found)"
echo ""
echo "🎯 Available VSCode tasks (18 total):"
echo "  📋 Build: Ctrl+Shift+P → 'Tasks: Run Build Task'"
echo "  🧪 Test:  Ctrl+Shift+P → 'Tasks: Run Test Task'"
echo "  📝 All:   Ctrl+Shift+P → 'Tasks: Run Task'"
echo ""
echo "🐛 Debugging:"
echo "  Press F5 to start debugging"
echo "  Available configurations: autoapp, unit tests"
echo ""
echo "📚 Documentation:"
echo "  - Quick start: docs/DEV_ENVIRONMENT_SUMMARY.md"
echo "  - Full docs:   docs/README.md"
echo "  - DevContainer: .devcontainer/README.md"
echo ""
echo "🚀 Ready to build! Try: 'Tasks: Run Build Task'"
echo ""
