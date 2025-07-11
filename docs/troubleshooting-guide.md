# OpenAuto Troubleshooting Guide

## Overview

This comprehensive troubleshooting guide helps diagnose and resolve issues with OpenAuto's modern architecture, including the logger, event bus, state machine, REST API, and traditional Android Auto functionality.

## Diagnostic Tools and Commands

### System Information Collection

#### Basic System Info
```bash
#!/bin/bash
# System diagnostic script
echo "=== OpenAuto System Diagnostics ==="
echo "Date: $(date)"
echo "Hostname: $(hostname)"
echo "OS: $(lsb_release -d | cut -f2)"
echo "Kernel: $(uname -r)"
echo "Architecture: $(uname -m)"
echo "Uptime: $(uptime)"
echo ""

echo "=== Hardware Information ==="
echo "CPU: $(lscpu | grep 'Model name' | cut -d: -f2 | xargs)"
echo "Memory: $(free -h | grep Mem: | awk '{print $2}')"
echo "Disk: $(df -h / | tail -1 | awk '{print $2, $3, $4}')"
echo ""

echo "=== OpenAuto Services ==="
systemctl status openauto --no-pager
systemctl status openauto-btservice --no-pager
echo ""

echo "=== USB Devices ==="
lsusb
echo ""

echo "=== Audio Devices ==="
aplay -l 2>/dev/null || echo "No audio devices found"
echo ""

echo "=== Network Interfaces ==="
ip addr show
```

#### Modern Architecture Status
```bash
#!/bin/bash
# Modern API diagnostic script
echo "=== Modern Architecture Status ==="

# Check REST API
echo "REST API Status:"
curl -s http://localhost:8080/api/v1/health 2>/dev/null | jq . || echo "REST API not responding"
echo ""

# Check configuration
echo "Configuration Status:"
curl -s http://localhost:8080/api/v1/config 2>/dev/null | jq '.status' || echo "Config API not responding"
echo ""

# Check event bus
echo "Event Bus Status:"
curl -s http://localhost:8080/api/v1/events/status 2>/dev/null | jq . || echo "Event API not responding"
echo ""

# Check state machine
echo "State Machine Status:"
curl -s http://localhost:8080/api/v1/state 2>/dev/null | jq . || echo "State API not responding"
echo ""

# Check logger
echo "Logger Status:"
curl -s http://localhost:8080/api/v1/logs?limit=5 2>/dev/null | jq . || echo "Logger API not responding"
```

### Log Analysis Tools

#### Real-time Log Monitoring
```bash
#!/bin/bash
# Multi-source log monitoring
echo "Starting OpenAuto log monitoring..."

# Function to monitor logs with color coding
monitor_logs() {
    tail -f /var/log/openauto/openauto.log \
         /var/log/openauto/btservice.log \
         /var/log/syslog | \
    while read line; do
        if [[ $line =~ ERROR|FATAL ]]; then
            echo -e "\033[31m$line\033[0m"  # Red
        elif [[ $line =~ WARN ]]; then
            echo -e "\033[33m$line\033[0m"  # Yellow
        elif [[ $line =~ INFO ]]; then
            echo -e "\033[32m$line\033[0m"  # Green
        elif [[ $line =~ DEBUG ]]; then
            echo -e "\033[36m$line\033[0m"  # Cyan
        else
            echo "$line"
        fi
    done
}

monitor_logs
```

#### Log Analysis Script
```bash
#!/bin/bash
# Log analysis and pattern detection
LOG_FILE="${1:-/var/log/openauto/openauto.log}"

echo "=== Log Analysis for $LOG_FILE ==="
echo ""

# Error summary
echo "Error Summary (last 24 hours):"
grep -i error "$LOG_FILE" | grep "$(date +%Y-%m-%d)" | \
    awk '{print $5}' | sort | uniq -c | sort -nr
echo ""

# Warning summary
echo "Warning Summary (last 24 hours):"
grep -i warn "$LOG_FILE" | grep "$(date +%Y-%m-%d)" | \
    awk '{print $5}' | sort | uniq -c | sort -nr
echo ""

# Performance issues
echo "Performance Issues (>1000ms):"
grep -i "performance.*[1-9][0-9][0-9][0-9]ms" "$LOG_FILE" | tail -10
echo ""

# Memory warnings
echo "Memory Warnings:"
grep -i "memory\|oom\|allocation" "$LOG_FILE" | tail -10
echo ""

# Network issues
echo "Network Issues:"
grep -i "network\|connection\|timeout" "$LOG_FILE" | tail -10
```

## Common Issues and Solutions

### 1. Service Startup Issues

#### Service Won't Start
**Symptoms:**
- `systemctl status openauto` shows failed state
- No response from REST API
- No log entries

**Diagnosis:**
```bash
# Check service status
sudo systemctl status openauto -l

# Check configuration syntax
sudo -u openauto /opt/openauto/autoapp --test-config

# Check file permissions
ls -la /opt/openauto/
ls -la /etc/openauto/
ls -la /var/lib/openauto/

# Check dependencies
ldd /opt/openauto/autoapp
```

**Solutions:**
```bash
# Fix permissions
sudo chown -R openauto:openauto /opt/openauto /var/lib/openauto /var/log/openauto
sudo chmod 755 /opt/openauto/autoapp

# Fix configuration
sudo nano /etc/openauto/openauto.conf
sudo systemctl restart openauto

# Install missing dependencies
sudo apt update && sudo apt install --fix-missing

# Check systemd service file
sudo systemctl daemon-reload
sudo systemctl enable openauto
```

#### Segmentation Fault on Startup
**Symptoms:**
- Core dump in logs
- Immediate service exit
- Signal 11 in systemctl status

**Diagnosis:**
```bash
# Enable core dumps
echo 'kernel.core_pattern=/tmp/core.%e.%p' | sudo tee -a /etc/sysctl.conf
sudo sysctl -p

# Run with gdb
sudo -u openauto gdb /opt/openauto/autoapp
(gdb) run --config /etc/openauto/openauto.conf
(gdb) bt full

# Check memory
valgrind --leak-check=full /opt/openauto/autoapp --test-mode
```

**Solutions:**
```bash
# Rebuild with debug symbols
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)

# Check for memory corruption
export MALLOC_CHECK_=2
/opt/openauto/autoapp --test-mode

# Update system libraries
sudo apt update && sudo apt upgrade
```

### 2. Modern Architecture Issues

#### REST API Not Responding
**Symptoms:**
- Port 8080 not listening
- Connection refused errors
- API endpoints return 404

**Diagnosis:**
```bash
# Check port binding
sudo netstat -tlnp | grep 8080
sudo ss -tlnp | grep 8080

# Check API configuration
grep -i "rest_api" /etc/openauto/openauto.conf

# Test direct connection
curl -v http://localhost:8080/api/v1/health
telnet localhost 8080
```

**Solutions:**
```bash
# Enable REST API in configuration
sudo sed -i 's/enable_rest_api = false/enable_rest_api = true/' /etc/openauto/openauto.conf
sudo systemctl restart openauto

# Check firewall
sudo ufw allow 8080/tcp
sudo iptables -I INPUT -p tcp --dport 8080 -j ACCEPT

# Bind to all interfaces
sudo sed -i 's/rest_api_bind = 127.0.0.1/rest_api_bind = 0.0.0.0/' /etc/openauto/openauto.conf
```

#### Event Bus Not Working
**Symptoms:**
- Events not propagating
- Components not responding to events
- Event API returns empty results

**Diagnosis:**
```bash
# Check event bus status
curl -s http://localhost:8080/api/v1/events/status | jq

# Test event posting
curl -X POST http://localhost:8080/api/v1/events \
  -H "Content-Type: application/json" \
  -d '{"type":"test","data":{"message":"hello"}}'

# Check event logs
grep -i "event\|bus" /var/log/openauto/openauto.log
```

**Solutions:**
```bash
# Enable event bus
sudo sed -i 's/enable_event_bus = false/enable_event_bus = true/' /etc/openauto/openauto.conf

# Restart service
sudo systemctl restart openauto

# Clear event queue
curl -X DELETE http://localhost:8080/api/v1/events/queue
```

#### Logger Issues
**Symptoms:**
- No log output
- Log files not created
- Performance degradation

**Diagnosis:**
```bash
# Check logger configuration
grep -i "log" /etc/openauto/openauto.conf
grep -i "log" /etc/openauto/logger.conf

# Test logger directly
/opt/openauto/logger_demo

# Check log file permissions
ls -la /var/log/openauto/

# Check disk space
df -h /var/log
```

**Solutions:**
```bash
# Create log directory
sudo mkdir -p /var/log/openauto
sudo chown openauto:openauto /var/log/openauto

# Fix log rotation
sudo logrotate -f /etc/logrotate.d/openauto

# Enable async logging
sudo sed -i 's/async_logging = false/async_logging = true/' /etc/openauto/logger.conf

# Reduce log level if performance issues
sudo sed -i 's/global = DEBUG/global = INFO/' /etc/openauto/logger.conf
```

### 3. Android Auto Connection Issues

#### USB Device Not Detected
**Symptoms:**
- `lsusb` doesn't show Android device
- No connection established
- Device charges but no data connection

**Diagnosis:**
```bash
# Check USB devices
lsusb -v
dmesg | grep -i usb | tail -20

# Check udev rules
ls -la /etc/udev/rules.d/*openauto*
udevadm test /sys/bus/usb/devices/*/

# Check Android device mode
adb devices
```

**Solutions:**
```bash
# Install udev rules
sudo cp configs/99-openauto.rules /etc/udev/rules.d/
sudo udevadm control --reload
sudo udevadm trigger

# Enable developer mode on Android
# Enable USB debugging
# Change USB mode to File Transfer

# Add user to groups
sudo usermod -a -G plugdev,dialout $USER

# Restart udev
sudo systemctl restart udev
```

#### WiFi Projection Not Working
**Symptoms:**
- Android device doesn't see WiFi network
- Connection established but no projection
- High latency or stuttering

**Diagnosis:**
```bash
# Check WiFi interface
iwconfig
ip addr show wlan0

# Check hostapd
sudo systemctl status hostapd
sudo journalctl -u hostapd

# Check dnsmasq
sudo systemctl status dnsmasq
sudo journalctl -u dnsmasq

# Test WiFi connection
sudo iw dev wlan0 station dump
```

**Solutions:**
```bash
# Configure hostapd
sudo nano /etc/hostapd/hostapd.conf
# Set correct interface and credentials

# Configure dnsmasq
sudo nano /etc/dnsmasq.conf
# Set DHCP range

# Enable IP forwarding
echo 'net.ipv4.ip_forward=1' | sudo tee -a /etc/sysctl.conf
sudo sysctl -p

# Configure iptables NAT
sudo iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
sudo iptables-save > /etc/iptables/rules.v4

# Restart services
sudo systemctl restart hostapd dnsmasq
```

### 4. Audio Issues

#### No Audio Output
**Symptoms:**
- Silent playback
- Audio device not found
- Crackling or distorted audio

**Diagnosis:**
```bash
# Check audio devices
aplay -l
pactl list sinks

# Test audio output
speaker-test -c 2 -t wav
aplay /usr/share/sounds/alsa/Front_Left.wav

# Check ALSA configuration
cat /proc/asound/cards
cat /proc/asound/devices

# Check PulseAudio
pulseaudio --check -v
pactl info
```

**Solutions:**
```bash
# Install audio packages
sudo apt install alsa-utils pulseaudio pulseaudio-utils

# Configure default audio device
echo 'defaults.pcm.card 0' | sudo tee -a /etc/asound.conf
echo 'defaults.ctl.card 0' | sudo tee -a /etc/asound.conf

# Restart audio services
sudo systemctl restart alsa-state
pulseaudio -k && pulseaudio --start

# Add user to audio group
sudo usermod -a -G audio openauto

# Set audio device in configuration
sudo sed -i 's/audio_output = .*/audio_output = pulse/' /etc/openauto/openauto.conf
```

#### Audio Latency Issues
**Symptoms:**
- Delayed audio
- Audio/video sync issues
- Stuttering playbook

**Diagnosis:**
```bash
# Check audio latency
pactl list sinks | grep -i latency

# Monitor audio performance
cat /proc/asound/card*/pcm*/sub*/status

# Check CPU usage during audio
top -p $(pgrep autoapp)
```

**Solutions:**
```bash
# Reduce audio buffer size
echo 'default-fragments = 2' | sudo tee -a /etc/pulse/daemon.conf
echo 'default-fragment-size-msec = 5' | sudo tee -a /etc/pulse/daemon.conf

# Optimize CPU governor
echo 'performance' | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# Use hardware audio if available
sudo sed -i 's/audio_output = pulse/audio_output = hw:0,0/' /etc/openauto/openauto.conf
```

### 5. Video Issues

#### No Video Display
**Symptoms:**
- Black screen
- Video device not found
- Graphics corruption

**Diagnosis:**
```bash
# Check video devices
ls -la /dev/dri/
ls -la /dev/fb*

# Check graphics drivers
lsmod | grep drm
dmesg | grep -i drm

# Test video output
cat /sys/class/drm/card*/status
xrandr --listmonitors

# Check OpenGL support
glxinfo | grep -i renderer
```

**Solutions:**
```bash
# Install graphics drivers
sudo apt install mesa-utils

# For Raspberry Pi
sudo raspi-config
# Advanced Options -> GL Driver -> GL (Full KMS)

# For Intel graphics
sudo apt install intel-media-va-driver

# For NVIDIA
sudo apt install nvidia-driver-470

# Configure video output
sudo sed -i 's/video_output = .*/video_output = drm/' /etc/openauto/openauto.conf

# Set proper permissions
sudo usermod -a -G video openauto
```

### 6. Bluetooth Issues

#### Bluetooth Service Not Starting
**Symptoms:**
- btservice fails to start
- No Bluetooth adapter found
- Pairing not working

**Diagnosis:**
```bash
# Check Bluetooth adapter
hciconfig
bluetoothctl show

# Check Bluetooth service
sudo systemctl status bluetooth
sudo journalctl -u bluetooth

# Check btservice
sudo systemctl status openauto-btservice
sudo journalctl -u openauto-btservice

# Test Bluetooth functionality
bluetoothctl scan on
```

**Solutions:**
```bash
# Install Bluetooth packages
sudo apt install bluez bluez-tools

# Enable Bluetooth service
sudo systemctl enable bluetooth
sudo systemctl start bluetooth

# Configure Bluetooth
sudo nano /etc/bluetooth/main.conf
# Set DiscoverableTimeout = 0
# Set PairableTimeout = 0

# Reset Bluetooth adapter
sudo hciconfig hci0 down
sudo hciconfig hci0 up

# Restart services
sudo systemctl restart bluetooth openauto-btservice
```

### 7. Performance Issues

#### High CPU Usage
**Symptoms:**
- System sluggish
- High load average
- Thermal throttling

**Diagnosis:**
```bash
# Monitor CPU usage
top -p $(pgrep autoapp)
htop

# Check CPU frequency
cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_cur_freq

# Profile application
perf record -g /opt/openauto/autoapp --test-mode
perf report

# Check thermal status
cat /sys/class/thermal/thermal_zone*/temp
```

**Solutions:**
```bash
# Optimize CPU governor
echo 'performance' | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# Reduce video quality
sudo sed -i 's/video_quality = high/video_quality = medium/' /etc/openauto/openauto.conf

# Enable hardware acceleration
sudo sed -i 's/enable_hw_accel = false/enable_hw_accel = true/' /etc/openauto/openauto.conf

# Limit process priority
sudo systemctl edit openauto
# Add:
# [Service]
# Nice=10
```

#### High Memory Usage
**Symptoms:**
- System running out of memory
- OOM killer activated
- Swap usage high

**Diagnosis:**
```bash
# Monitor memory usage
free -h
cat /proc/meminfo

# Check process memory
ps aux --sort=-rss | head -20
pmap $(pgrep autoapp)

# Check for memory leaks
valgrind --leak-check=full /opt/openauto/autoapp --test-mode
```

**Solutions:**
```bash
# Limit memory usage
sudo systemctl edit openauto
# Add:
# [Service]
# MemoryMax=1G

# Enable swap if not available
sudo fallocate -l 1G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile

# Optimize logger queue size
sudo sed -i 's/max_queue_size = 10000/max_queue_size = 5000/' /etc/openauto/logger.conf

# Enable asynchronous logging
sudo sed -i 's/async_logging = false/async_logging = true/' /etc/openauto/logger.conf
```

## Advanced Troubleshooting

### Network Debugging

#### REST API Debug Mode
```bash
# Enable debug logging for REST API
curl -X PUT http://localhost:8080/api/v1/logs/level \
  -H "Content-Type: application/json" \
  -d '{"level":"DEBUG","category":"API"}'

# Monitor API calls
tcpdump -i lo port 8080

# Check API performance
ab -n 100 -c 10 http://localhost:8080/api/v1/health
```

#### Network Tracing
```bash
# Trace network calls
strace -e trace=network -p $(pgrep autoapp)

# Monitor network connections
netstat -tuln | grep autoapp
ss -tuln | grep autoapp

# Check for network timeouts
ping -c 5 8.8.8.8
traceroute 8.8.8.8
```

### Database Issues

#### Configuration Database Corruption
```bash
# Check database integrity
sqlite3 /var/lib/openauto/config.db "PRAGMA integrity_check;"

# Backup and repair
cp /var/lib/openauto/config.db /var/lib/openauto/config.db.backup
sqlite3 /var/lib/openauto/config.db ".dump" | sqlite3 /var/lib/openauto/config_new.db
mv /var/lib/openauto/config_new.db /var/lib/openauto/config.db

# Reset to defaults if needed
rm /var/lib/openauto/config.db
sudo systemctl restart openauto
```

### Modern Logger Deep Debugging

#### Logger Performance Analysis
```bash
# Monitor logger queue
watch -n 1 'curl -s http://localhost:8080/api/v1/logs/stats | jq'

# Check log file sizes
du -sh /var/log/openauto/*

# Monitor log write performance
iostat -x 1 /var/log

# Analyze log patterns
awk '{print $3}' /var/log/openauto/openauto.log | sort | uniq -c | sort -nr
```

#### Logger Memory Debugging
```bash
# Check logger memory usage
pmap $(pgrep autoapp) | grep -i log

# Monitor memory allocation
strace -e trace=mmap,munmap -p $(pgrep autoapp) 2>&1 | grep -i log

# Analyze memory leaks in logger
valgrind --tool=memcheck --leak-check=full --track-origins=yes /opt/openauto/logger_demo
```

## Recovery Procedures

### Emergency Recovery

#### Service Recovery
```bash
#!/bin/bash
# Emergency service recovery script

echo "Starting emergency recovery..."

# Stop all services
sudo systemctl stop openauto openauto-btservice

# Kill any remaining processes
sudo pkill -f autoapp
sudo pkill -f btservice

# Clear temporary files
sudo rm -rf /tmp/openauto*
sudo rm -rf /var/run/openauto*

# Reset permissions
sudo chown -R openauto:openauto /opt/openauto /var/lib/openauto /var/log/openauto
sudo chmod -R 755 /opt/openauto
sudo chmod -R 644 /etc/openauto/*

# Restart services
sudo systemctl daemon-reload
sudo systemctl start openauto openauto-btservice

echo "Recovery complete. Check service status:"
sudo systemctl status openauto openauto-btservice
```

#### Configuration Reset
```bash
#!/bin/bash
# Reset configuration to defaults

echo "Resetting configuration to defaults..."

# Backup current configuration
sudo cp -r /etc/openauto /etc/openauto.backup.$(date +%s)
sudo cp -r /var/lib/openauto /var/lib/openauto.backup.$(date +%s)

# Stop service
sudo systemctl stop openauto

# Remove configuration and data
sudo rm -rf /var/lib/openauto/*
sudo rm /etc/openauto/openauto.conf

# Copy default configuration
sudo cp /opt/openauto/configs/openauto.conf.default /etc/openauto/openauto.conf
sudo chown root:openauto /etc/openauto/openauto.conf
sudo chmod 640 /etc/openauto/openauto.conf

# Start service
sudo systemctl start openauto

echo "Configuration reset complete."
```

### Data Recovery

#### Log Recovery
```bash
#!/bin/bash
# Recover logs from journal if log files are corrupted

echo "Recovering logs from systemd journal..."

# Export service logs
sudo journalctl -u openauto --since "1 week ago" > /tmp/openauto_recovered.log
sudo journalctl -u openauto-btservice --since "1 week ago" > /tmp/btservice_recovered.log

# Compress and store
sudo gzip /tmp/*_recovered.log
sudo mv /tmp/*_recovered.log.gz /var/log/openauto/

echo "Log recovery complete."
```

This troubleshooting guide provides comprehensive diagnostic procedures and solutions for all aspects of the OpenAuto modern architecture. Use the diagnostic scripts and step-by-step solutions to identify and resolve issues quickly.
