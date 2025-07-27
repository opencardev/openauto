#!/bin/bash

# OpenAuto Debug Logging Configuration Script
# Run this before starting autoapp to enable comprehensive debug logging

echo "🔧 Configuring OpenAuto for Debug Logging..."

# Create debug configuration directory
mkdir -p ~/.config/openauto

# Create debug logging configuration
cat > ~/.config/openauto/debug-logging.conf << 'EOF'
# OpenAuto Debug Logging Configuration

# Global log level (TRACE, DEBUG, INFO, WARN, ERROR, FATAL)
global_log_level=DEBUG

# Category-specific log levels
log_level_android_auto=DEBUG
log_level_ui=DEBUG
log_level_camera=DEBUG
log_level_network=DEBUG
log_level_bluetooth=DEBUG
log_level_audio=DEBUG
log_level_video=DEBUG
log_level_config=DEBUG
log_level_api=DEBUG
log_level_event=DEBUG
log_level_state=DEBUG
log_level_system=DEBUG

# AASDK debug settings
aasdk_debug=true
aasdk_log_level=DEBUG

# Enable detailed logging
show_thread_id=true
show_location=true
show_context=true
use_colors=true

# Log to file as well as console
log_to_file=true
log_file_path=/tmp/openauto-debug.log
EOF

echo "✅ Debug logging configuration created at ~/.config/openauto/debug-logging.conf"

# Set environment variables for this session
export OPENAUTO_LOG_LEVEL=DEBUG
export AASDK_LOG_LEVEL=DEBUG
export OPENAUTO_DEBUG_MODE=1

echo "🔍 Environment variables set:"
echo "   OPENAUTO_LOG_LEVEL=DEBUG"
echo "   AASDK_LOG_LEVEL=DEBUG" 
echo "   OPENAUTO_DEBUG_MODE=1"

# Create boost log configuration for legacy components
cat > openauto-logs.ini << 'EOF'
[Core]
Filter="Severity >= debug"
Format="[%TimeStamp%] [%Severity%] [%Channel%] %Message%"

[Sinks.1]
Destination=Console
Format="[%TimeStamp%] [%Severity%] [%Channel%] %Message%"
Filter="Severity >= debug"
EOF

echo "✅ Boost log configuration created: openauto-logs.ini"

echo "🚀 Debug logging is now configured. Start autoapp to see detailed logs."
echo ""
echo "📝 Usage:"
echo "   ./configure_debug_logging.sh    # Run this script"
echo "   ./autoapp                       # Start autoapp with debug logging"
echo ""
echo "📊 Log files:"
echo "   Console output: Real-time debug logs"
echo "   /tmp/openauto-debug.log: Detailed log file"
echo "   autoapp.log: Application-specific logs"
