#!/bin/bash

echo "=== OpenAuto Logging Fix ==="
echo "The issue: autoapp isn't reading openauto.conf for logging configuration"
echo ""

# 1. Create the log directory with proper permissions
echo "1. Creating log directory..."
sudo mkdir -p /var/log/openauto
sudo chown pi:pi /var/log/openauto
sudo chmod 755 /var/log/openauto

# 2. Check current configuration files
echo ""
echo "2. Checking configuration files:"
if [ -f "bin/openauto.conf" ]; then
    echo "   ✅ bin/openauto.conf found (modernization config - not used by legacy autoapp)"
else
    echo "   ❌ bin/openauto.conf not found"
fi

if [ -f "bin/openauto.ini" ]; then
    echo "   ✅ bin/openauto.ini found (legacy config - used by autoapp)"
else
    echo "   ❌ bin/openauto.ini not found (will be created)"
fi

# 3. Create/update openauto.ini for legacy configuration system
echo ""
echo "3. Creating/updating openauto.ini for legacy system..."

cat > bin/openauto.ini << 'EOF'
[General]
HandednessOfTrafficType=0
ShowClock=true
ShowBigClock=false
OldGUI=false
AlphaTrans=50
HideMenuToggle=false
HideAlpha=false
ShowLux=false
ShowCursor=false
HideBrightnessControl=false
ShowNetworkinfo=false
HideWarning=false

[Video]
FPS=30
Resolution=1
ScreenDPI=140
OMXLayerIndex=1
MarginWidth=0
MarginHeight=0

[Input]
EnableTouchscreen=true
EnablePlayerControl=false

[Bluetooth]
AdapterType=0
AdapterAddress=
WirelessProjectionEnabled=true

[Audio]
ChannelMediaEnabled=true
ChannelGuidanceEnabled=true
ChannelSystemEnabled=true
ChannelTelephonyEnabled=true
OutputBackendType=0
EOF

echo "   ✅ bin/openauto.ini created/updated"

# 4. Set environment variables for current session
echo ""
echo "4. Setting environment variables for debug logging..."
export OPENAUTO_LOG_LEVEL=DEBUG
export OPENAUTO_DEBUG_MODE=1
export BOOST_LOG_TRIVIAL_VERBOSITY=debug

echo "   ✅ Environment variables set:"
echo "      OPENAUTO_LOG_LEVEL=DEBUG"
echo "      OPENAUTO_DEBUG_MODE=1" 
echo "      BOOST_LOG_TRIVIAL_VERBOSITY=debug"

# 5. Create a custom logging configuration for boost log
echo ""
echo "5. Creating boost log configuration..."

cat > openauto-logs.ini << 'EOF'
[Core]
Filter="Severity >= debug"

[Sinks.1]
Destination=Console
Format="[%TimeStamp%] [%Severity%] [%Channel%] %Message%"
Filter="Severity >= debug"

[Sinks.2]
Destination=TextFile
FileName="/var/log/openauto/openauto-%Y%m%d.log"
Format="[%TimeStamp%] [%Severity%] [%Channel%] %Message%"
Filter="Severity >= debug"
Append=true
AutoFlush=true
EOF

echo "   ✅ openauto-logs.ini created for boost logging"

# 6. Create a manual file logging wrapper
echo ""
echo "6. Creating manual file logging wrapper..."

cat > start_autoapp_with_logging.sh << 'EOF'
#!/bin/bash

# OpenAuto with comprehensive logging
LOG_FILE="/var/log/openauto/openauto.log"

echo "=== OpenAuto Debug Session Started at $(date) ===" | tee -a "$LOG_FILE"

# Set debug environment
export OPENAUTO_LOG_LEVEL=DEBUG
export OPENAUTO_DEBUG_MODE=1
export BOOST_LOG_TRIVIAL_VERBOSITY=debug

# Start autoapp with output to both console and file
./autoapp 2>&1 | tee -a "$LOG_FILE"

echo "=== OpenAuto Debug Session Ended at $(date) ===" | tee -a "$LOG_FILE"
EOF

chmod +x start_autoapp_with_logging.sh

echo "   ✅ start_autoapp_with_logging.sh created"

# 7. Test the setup
echo ""
echo "7. Testing logging setup..."
echo "$(date): Logging fix test entry" >> /var/log/openauto/openauto.log

if [ -w "/var/log/openauto/openauto.log" ]; then
    echo "   ✅ Log file is writable"
else
    echo "   ❌ Log file permission issue"
fi

echo ""
echo "=== Fix Complete ==="
echo ""
echo "📝 Usage options:"
echo "   1. Quick test:    OPENAUTO_DEBUG_MODE=1 ./autoapp"
echo "   2. With logging:  ./start_autoapp_with_logging.sh" 
echo "   3. Monitor logs:  tail -f /var/log/openauto/openauto.log"
echo ""
echo "📊 Configuration:"
echo "   • Legacy config:  bin/openauto.ini (used by autoapp)"
echo "   • Modern config:  bin/openauto.conf (for future modernization)"
echo "   • Boost config:   openauto-logs.ini (for detailed logging)"
echo "   • Log file:       /var/log/openauto/openauto.log"
