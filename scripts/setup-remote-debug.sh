#!/bin/bash
# Setup script for remote debugging on Raspberry Pi
# Run this on the Raspberry Pi to prepare for remote debugging

set -e

echo "=== OpenAuto Remote Debugging Setup ==="
echo ""

# Detect if running as sudo and get the real user
if [ "$EUID" -eq 0 ] && [ -n "$SUDO_USER" ]; then
    REAL_USER="$SUDO_USER"
    REAL_HOME=$(eval echo "~$SUDO_USER")
    echo "⚠️  Running as sudo. Using $REAL_USER's home directory: $REAL_HOME"
else
    REAL_USER="$USER"
    REAL_HOME="$HOME"
fi

# Check if this is a fresh setup
FRESH_SETUP=false
if [ ! -d "$REAL_HOME/src/openauto" ]; then
    FRESH_SETUP=true
fi

# Check if gdbserver is installed
if ! command -v gdbserver &> /dev/null; then
    echo "Installing gdbserver..."
    sudo apt-get update
    sudo apt-get install -y gdbserver git cmake build-essential pkg-config
else
    echo "✓ gdbserver is already installed"
fi

# If fresh setup, offer to clone and build openauto
if [ "$FRESH_SETUP" = true ]; then
    echo ""
    echo "OpenAuto source not found in $REAL_HOME/src/openauto"
    read -p "Do you want to clone and build OpenAuto with debug symbols? [Y/n]: " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]] || [[ -z $REPLY ]]; then
        echo ""
        echo "Cloning OpenAuto repository..."
        mkdir -p "$REAL_HOME/src"
        cd "$REAL_HOME/src"
        git clone https://github.com/opencardev/openauto.git
        cd openauto
        git checkout develop
        
        echo ""
        echo "Installing build dependencies..."
        sudo apt-get install -y \
            libboost-all-dev \
            libusb-1.0-0-dev \
            libssl-dev \
            libprotobuf-dev \
            protobuf-compiler \
            libqt5multimedia5 \
            libqt5multimedia5-plugins \
            libqt5multimediawidgets5 \
            qtmultimedia5-dev \
            libqt5bluetooth5 \
            libqt5bluetooth5-bin \
            qtconnectivity5-dev \
            librtaudio-dev \
            libgstreamer1.0-dev \
            libgstreamer-plugins-base1.0-dev \
            gstreamer1.0-plugins-good \
            gstreamer1.0-plugins-bad \
            gstreamer1.0-plugins-ugly \
            gstreamer1.0-libav \
            gstreamer1.0-alsa \
            libgps-dev \
            gpsd \
            libblkid-dev \
            libtag1-dev
        
        # Try to install aasdk from apt first
        if sudo apt-cache show libaasdk-dev &> /dev/null; then
            echo "Installing aasdk from apt..."
            sudo apt-get install -y libaasdk-dev
        else
            echo "Building aasdk from source..."
            cd "$REAL_HOME/src"
            git clone https://github.com/opencardev/aasdk.git
            cd aasdk
            mkdir -p build-release
            cd build-release
            cmake -DCMAKE_BUILD_TYPE=Release ..
            make -j$(nproc)
            sudo make install
            sudo ldconfig
        fi
        
        echo ""
        echo "Building OpenAuto with debug symbols..."
        cd "$REAL_HOME/src/openauto"
        chmod +x build.sh
        sudo -u "$REAL_USER" ./build.sh debug
        
        echo ""
        echo "✓ OpenAuto built successfully with debug symbols"
        echo "  Binary location: $REAL_HOME/src/openauto/build-debug/autoapp"
    fi
fi

# Stop the running service
echo ""
echo "Stopping openauto service..."
sudo systemctl stop openauto || true

# Find the autoapp binary
AUTOAPP_PATH="/opt/crankshaft/autoapp"
if [ ! -f "$AUTOAPP_PATH" ]; then
    echo "autoapp not found at $AUTOAPP_PATH"
    
    # Try the build directory
    if [ -f "$REAL_HOME/src/openauto/build-debug/autoapp" ]; then
        AUTOAPP_PATH="$REAL_HOME/src/openauto/build-debug/autoapp"
        echo "✓ Using debug build: $AUTOAPP_PATH"
    else
        echo "Searching for autoapp..."
        AUTOAPP_PATH=$(which autoapp 2>/dev/null || echo "")
        if [ -z "$AUTOAPP_PATH" ]; then
            echo "Error: Could not find autoapp binary"
            echo ""
            echo "Please ensure OpenAuto is built with debug symbols:"
            echo "  cd $REAL_HOME/src/openauto"
            echo "  ./build.sh debug"
            exit 1
        fi
        echo "✓ Found autoapp at: $AUTOAPP_PATH"
    fi
else
    echo "✓ Found autoapp at: $AUTOAPP_PATH"
fi

# Check if binary has debug symbols
echo ""
echo "Checking for debug symbols..."
if file "$AUTOAPP_PATH" | grep -q "not stripped"; then
    echo "✓ Debug symbols present"
else
    echo "⚠ Warning: Binary appears to be stripped (no debug symbols)"
    echo "  Debugging will be limited. Consider rebuilding with -DCMAKE_BUILD_TYPE=Debug"
fi

# Kill any existing gdbserver
echo ""
echo "Cleaning up any existing gdbserver processes..."
sudo pkill -9 gdbserver 2>/dev/null || true

# Display options
echo ""
echo "=== Debug Options ==="
echo ""
echo "Choose how to start debugging:"
echo "1) Start autoapp under gdbserver (recommended for reproducing hang)"
echo "2) Attach gdbserver to running autoapp"
echo "3) Just prepare (manual start)"
echo ""
read -p "Enter choice [1-3]: " choice

case $choice in
    1)
        echo ""
        echo "Starting autoapp under gdbserver on port 2345..."
        echo "The application will wait for debugger to connect."
        echo ""
        echo "In VS Code:"
        echo "  1. Press F5"
        echo "  2. Select 'Remote Debug on rpi5 (Launch)'"
        echo "  3. The application will start and you can set breakpoints"
        echo ""
        echo "Starting gdbserver..."
        sudo gdbserver :2345 "$AUTOAPP_PATH"
        ;;
    2)
        echo ""
        echo "Starting autoapp normally..."
        sudo "$AUTOAPP_PATH" &
        AUTOAPP_PID=$!
        sleep 2
        
        if ps -p $AUTOAPP_PID > /dev/null; then
            echo "✓ autoapp started with PID: $AUTOAPP_PID"
            echo ""
            echo "Attaching gdbserver to PID $AUTOAPP_PID on port 2345..."
            echo ""
            echo "In VS Code:"
            echo "  1. Press F5"
            echo "  2. Select 'Remote Debug on rpi5 (Attach)'"
            echo "  3. You can now debug the running process"
            echo ""
            sudo gdbserver --attach :2345 $AUTOAPP_PID
        else
            echo "Error: Failed to start autoapp"
            exit 1
        fi
        ;;
    3)
        echo ""
        echo "=== Manual Start Instructions ==="
        echo ""
        echo "To start with gdbserver:"
        echo "  sudo gdbserver :2345 $AUTOAPP_PATH"
        echo ""
        echo "To attach to running process:"
        echo "  1. Start autoapp: sudo $AUTOAPP_PATH &"
        echo "  2. Get PID: ps aux | grep autoapp"
        echo "  3. Attach: sudo gdbserver --attach :2345 PID"
        echo ""
        echo "Then in VS Code, press F5 and select the remote debug configuration"
        ;;
    *)
        echo "Invalid choice"
        exit 1
        ;;
esac
