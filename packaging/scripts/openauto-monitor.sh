#!/bin/bash
# OpenAuto System Monitoring Script

# Configuration
LOG_FILE="/var/log/openauto/monitor.log"
CHECK_INTERVAL=60
MEMORY_THRESHOLD=80
CPU_THRESHOLD=90

log_message() {
    echo "$(date '+%Y-%m-%d %H:%M:%S'): $1" >> "$LOG_FILE"
}

check_service_status() {
    local service="$1"
    if ! systemctl is-active --quiet "$service"; then
        log_message "ERROR: Service $service is not running"
        return 1
    fi
    return 0
}

check_memory_usage() {
    local process="$1"
    local mem_usage=$(ps -o pid,ppid,user,%mem,command -ax | grep "$process" | grep -v grep | awk '{print $4}' | head -1)
    
    if [ -n "$mem_usage" ]; then
        if (( $(echo "$mem_usage > $MEMORY_THRESHOLD" | bc -l) )); then
            log_message "WARNING: High memory usage for $process: ${mem_usage}%"
            return 1
        fi
    fi
    return 0
}

check_api_health() {
    local response=$(curl -s -f http://localhost:8080/api/v1/health 2>/dev/null)
    if [ $? -ne 0 ]; then
        log_message "ERROR: REST API not responding"
        return 1
    fi
    return 0
}

check_usb_devices() {
    local usb_count=$(lsusb | grep -E "(Google|Android|Samsung|LG|HTC|Motorola)" | wc -l)
    if [ "$usb_count" -eq 0 ]; then
        log_message "INFO: No Android devices detected via USB"
    else
        log_message "INFO: $usb_count Android device(s) detected"
    fi
}

main() {
    log_message "Starting OpenAuto monitoring"
    
    while true; do
        # Check services
        check_service_status "openauto"
        check_service_status "openauto-btservice"
        
        # Check resource usage
        check_memory_usage "autoapp"
        check_memory_usage "btservice"
        
        # Check API health
        if systemctl is-active --quiet openauto; then
            check_api_health
        fi
        
        # Check USB devices (less frequent)
        if [ $(($(date +%s) % 300)) -eq 0 ]; then
            check_usb_devices
        fi
        
        sleep "$CHECK_INTERVAL"
    done
}

# Check if running as root or openauto user
if [ "$EUID" -ne 0 ] && [ "$(whoami)" != "openauto" ]; then
    echo "This script should be run as root or the openauto user"
    exit 1
fi

# Create log file if it doesn't exist
touch "$LOG_FILE"

# Start monitoring
main
