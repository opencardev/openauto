# OpenAuto Deployment Guide

## Overview

This guide covers deploying OpenAuto with the modern architecture in various environments, from development setups to production automotive systems.

## Deployment Architectures

### 1. Standalone Mode
- Single device running all components
- Suitable for development and simple setups
- Minimal resource requirements

### 2. Distributed Mode
- Components spread across multiple devices
- REST API for remote communication
- Event bus for inter-component messaging
- Suitable for complex automotive systems

### 3. Container Mode
- Docker/Podman containerized deployment
- Kubernetes orchestration support
- Scalable and maintainable
- Cloud and edge deployment ready

## Pre-Deployment Checklist

### System Requirements

#### Minimum Requirements
- **CPU**: ARM Cortex-A7 1GHz+ or x86_64 1.5GHz+
- **RAM**: 512MB (1GB recommended)
- **Storage**: 2GB free space
- **USB**: USB 2.0+ for Android Auto
- **Network**: WiFi or Ethernet for remote features

#### Recommended Requirements
- **CPU**: ARM Cortex-A72 1.5GHz+ or x86_64 2.5GHz+
- **RAM**: 2GB+ (4GB for development)
- **Storage**: 8GB+ SSD/eMMC
- **USB**: USB 3.0+ for better performance
- **GPU**: Hardware video acceleration support

### Operating System Support

#### Production Ready
- **Raspberry Pi OS**: Bullseye (11) or newer
- **Ubuntu**: 20.04 LTS or newer
- **Debian**: 11 (Bullseye) or newer
- **Yocto**: Custom automotive distributions

#### Development/Testing
- **Arch Linux**: Rolling release
- **Fedora**: Recent versions
- **OpenWrt**: For embedded systems
- **Windows**: WSL2 for development

## Configuration Management

### Configuration Files

#### Main Configuration (`/etc/openauto/openauto.conf`)
```ini
[system]
# System-wide settings
log_level = INFO
data_directory = /var/lib/openauto
temp_directory = /tmp/openauto
user = openauto
group = openauto

[android_auto]
# Android Auto settings
usb_hotplug = true
wifi_projection = true
audio_output = pulse
video_output = drm

[modern_api]
# Modern architecture settings
enable_rest_api = true
rest_api_port = 8080
rest_api_bind = 0.0.0.0
enable_event_bus = true
enable_state_machine = true

[logging]
# Modern logger settings
async_logging = true
max_queue_size = 10000
console_output = true
file_output = true
file_path = /var/log/openauto/openauto.log
file_max_size = 50MB
file_rotation_count = 5
json_format = false
```

#### Logger Configuration (`/etc/openauto/logger.conf`)
```ini
[levels]
# Global and category-specific log levels
global = INFO
system = DEBUG
android_auto = INFO
ui = WARN
camera = ERROR
network = DEBUG
bluetooth = INFO
audio = WARN
video = INFO

[sinks]
# Output destinations
console = true
file = true
remote = false
syslog = true

[formatters]
# Output formats
console_format = console
file_format = detailed
remote_format = json

[performance]
# Performance logging
enable_performance_logging = true
performance_threshold_ms = 100
```

#### Service Configuration (`/etc/openauto/services.conf`)
```ini
[autoapp]
enabled = true
auto_start = true
restart_policy = always
user = openauto
working_directory = /opt/openauto

[btservice]
enabled = true
auto_start = true
bluetooth_adapter = hci0
discoverable = true
pairable = true

[rest_api]
enabled = true
port = 8080
cors_enabled = true
auth_required = false
rate_limiting = true
```

### Environment Variables

```bash
# System paths
export OPENAUTO_HOME="/opt/openauto"
export OPENAUTO_CONFIG="/etc/openauto"
export OPENAUTO_DATA="/var/lib/openauto"
export OPENAUTO_LOGS="/var/log/openauto"

# Runtime settings
export OPENAUTO_LOG_LEVEL="INFO"
export OPENAUTO_ENABLE_MODERN_API="true"
export OPENAUTO_REST_API_PORT="8080"

# Hardware settings
export OPENAUTO_AUDIO_DEVICE="default"
export OPENAUTO_VIDEO_DEVICE="/dev/dri/card0"
export OPENAUTO_USB_DEVICE="auto"
```

## Package Building and Testing

### Build Debian Package

#### Prerequisites
```bash
# Install build dependencies
sudo apt update
sudo apt install \
    build-essential cmake pkg-config git \
    libboost-all-dev libprotobuf-dev protobuf-compiler \
    libssl-dev libusb-1.0-0-dev libudev-dev \
    qtbase5-dev qtmultimedia5-dev qtconnectivity5-dev \
    libasound2-dev libpulse-dev librtaudio-dev \
    nlohmann-json3-dev libevent-dev
```

#### Build Process
```bash
# Clone and build
git clone https://github.com/opencardev/openauto.git
cd openauto

# Build package
./build-package.sh

# Install package
sudo apt install ./build-package/openauto-modern*.deb
```

#### Package Testing
```bash
# Test installation
sudo systemctl status openauto
sudo systemctl status openauto-btservice

# Test configuration
cat /etc/openauto/openauto.conf

# Test REST API
curl http://localhost:8080/api/v1/health

# Test logging
tail -f /var/log/openauto/openauto.log

# Test removal
sudo apt remove openauto-modern

# Test purge
sudo apt purge openauto-modern
```

#### Package Information
```bash
# View package details
dpkg -l | grep openauto
dpkg -L openauto-modern
dpkg -s openauto-modern

# Check dependencies
apt-cache depends openauto-modern
```

## Installation Methods

### 1. Package Installation (Recommended)

#### Local Debian Package
```bash
# Build the package from source
./build-package.sh

# Install the generated package
sudo apt install ./build-package/openauto-modern*.deb

# Services will start automatically
sudo systemctl status openauto
sudo systemctl status openauto-btservice
```

#### Debian/Ubuntu Package (Future)
```bash
# Add repository (when available)
wget -qO - https://repo.opencardev.com/key.asc | sudo apt-key add -
echo "deb https://repo.opencardev.com/debian $(lsb_release -cs) main" | sudo tee /etc/apt/sources.list.d/openauto.list

# Install
sudo apt update
sudo apt install openauto-modern

# Start services (auto-enabled)
sudo systemctl status openauto
sudo systemctl status openauto-btservice
```

#### RPM Package (Fedora/RHEL)
```bash
# Add repository
sudo dnf config-manager --add-repo https://repo.opencardev.com/fedora/openauto.repo

# Install
sudo dnf install openauto-modern

# Start services
sudo systemctl enable --now openauto
```

### 2. Manual Installation

#### Create User and Directories
```bash
# Create system user
sudo useradd --system --home /opt/openauto --shell /bin/false openauto

# Create directories
sudo mkdir -p /opt/openauto
sudo mkdir -p /etc/openauto
sudo mkdir -p /var/lib/openauto
sudo mkdir -p /var/log/openauto

# Set permissions
sudo chown -R openauto:openauto /opt/openauto /var/lib/openauto /var/log/openauto
sudo chmod 755 /opt/openauto /var/lib/openauto
sudo chmod 750 /var/log/openauto
```

#### Install Binaries
```bash
# Copy binaries
sudo cp build/autoapp /opt/openauto/
sudo cp build/btservice /opt/openauto/
sudo cp build/logger_demo /opt/openauto/

# Set permissions
sudo chown openauto:openauto /opt/openauto/*
sudo chmod 755 /opt/openauto/*
```

#### Install Configuration
```bash
# Copy configuration files
sudo cp configs/openauto.conf /etc/openauto/
sudo cp configs/logger.conf /etc/openauto/
sudo cp configs/services.conf /etc/openauto/

# Set permissions
sudo chown root:openauto /etc/openauto/*
sudo chmod 640 /etc/openauto/*
```

### 3. Container Installation

#### Docker Compose
```yaml
# docker-compose.yml
version: '3.8'

services:
  openauto:
    image: opencardev/openauto:latest
    container_name: openauto
    restart: unless-stopped
    privileged: true
    network_mode: host
    volumes:
      - /dev:/dev
      - /run/udev:/run/udev:ro
      - openauto-data:/var/lib/openauto
      - openauto-logs:/var/log/openauto
      - ./config:/etc/openauto:ro
    environment:
      - OPENAUTO_LOG_LEVEL=INFO
      - OPENAUTO_ENABLE_MODERN_API=true
    ports:
      - "8080:8080"
    devices:
      - /dev/bus/usb:/dev/bus/usb
      - /dev/dri:/dev/dri
    
  openauto-btservice:
    image: opencardev/openauto-btservice:latest
    container_name: openauto-btservice
    restart: unless-stopped
    network_mode: host
    privileged: true
    volumes:
      - /var/run/dbus:/var/run/dbus
      - ./config:/etc/openauto:ro
    devices:
      - /dev/rfkill:/dev/rfkill

volumes:
  openauto-data:
  openauto-logs:
```

#### Deploy with Docker Compose
```bash
# Create deployment directory
mkdir -p ~/openauto-deploy/config
cd ~/openauto-deploy

# Copy configuration
cp /path/to/configs/* config/

# Deploy
docker-compose up -d

# Check logs
docker-compose logs -f openauto
```

## Service Management

### Systemd Services

#### Main Service (`/etc/systemd/system/openauto.service`)
```ini
[Unit]
Description=OpenAuto Android Auto Implementation
Documentation=https://github.com/opencardev/openauto
After=network.target sound.target
Wants=network.target
RequiredBy=openauto-btservice.service

[Service]
Type=simple
User=openauto
Group=openauto
WorkingDirectory=/opt/openauto
ExecStart=/opt/openauto/autoapp --config /etc/openauto/openauto.conf
ExecReload=/bin/kill -HUP $MAINPID
Restart=always
RestartSec=5
StandardOutput=journal
StandardError=journal
Environment=OPENAUTO_HOME=/opt/openauto
Environment=OPENAUTO_CONFIG=/etc/openauto
MemoryMax=1G
TasksMax=100

# Security settings
NoNewPrivileges=true
ProtectSystem=strict
ProtectHome=true
ReadWritePaths=/var/lib/openauto /var/log/openauto /tmp
PrivateTmp=true

[Install]
WantedBy=multi-user.target
```

#### Bluetooth Service (`/etc/systemd/system/openauto-btservice.service`)
```ini
[Unit]
Description=OpenAuto Bluetooth Service
Documentation=https://github.com/opencardev/openauto
After=bluetooth.target dbus.service
Requires=bluetooth.target dbus.service
PartOf=openauto.service

[Service]
Type=simple
User=root
WorkingDirectory=/opt/openauto
ExecStart=/opt/openauto/btservice --config /etc/openauto/services.conf
Restart=always
RestartSec=3
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
```

#### Service Operations
```bash
# Enable and start services
sudo systemctl daemon-reload
sudo systemctl enable openauto openauto-btservice
sudo systemctl start openauto openauto-btservice

# Check status
sudo systemctl status openauto
sudo systemctl status openauto-btservice

# View logs
sudo journalctl -u openauto -f
sudo journalctl -u openauto-btservice -f

# Restart services
sudo systemctl restart openauto
sudo systemctl restart openauto-btservice
```

## Network Configuration

### Firewall Settings

#### UFW (Ubuntu)
```bash
# Allow REST API
sudo ufw allow 8080/tcp comment "OpenAuto REST API"

# Allow development access
sudo ufw allow from 192.168.1.0/24 to any port 8080

# Enable firewall
sudo ufw enable
```

#### iptables
```bash
# Allow REST API
sudo iptables -A INPUT -p tcp --dport 8080 -j ACCEPT

# Save rules
sudo iptables-save > /etc/iptables/rules.v4
```

### WiFi Hotspot (for WiFi Projection)

#### hostapd Configuration (`/etc/hostapd/hostapd.conf`)
```ini
interface=wlan0
driver=nl80211
ssid=OpenAuto-WiFi
hw_mode=g
channel=7
wmm_enabled=0
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=OpenAuto123
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP
```

#### dnsmasq Configuration (`/etc/dnsmasq.conf`)
```ini
interface=wlan0
dhcp-range=192.168.4.2,192.168.4.20,255.255.255.0,24h
```

## Security Configuration

### User Permissions

#### udev Rules (`/etc/udev/rules.d/99-openauto.rules`)
```bash
# USB devices for Android Auto
SUBSYSTEM=="usb", ATTRS{idVendor}=="18d1", GROUP="openauto", MODE="0664"
SUBSYSTEM=="usb", ATTRS{idVendor}=="0bb4", GROUP="openauto", MODE="0664"

# Audio devices
SUBSYSTEM=="sound", GROUP="audio", MODE="0664"
KERNEL=="controlC[0-9]*", GROUP="audio", MODE="0664"

# Video devices
SUBSYSTEM=="drm", GROUP="video", MODE="0664"
KERNEL=="card[0-9]*", GROUP="video", MODE="0664"
```

#### Group Membership
```bash
# Add user to required groups
sudo usermod -a -G audio,video,bluetooth,dialout openauto

# Add current user for development
sudo usermod -a -G openauto $USER
```

### SSL/TLS Configuration

#### Generate Certificates
```bash
# Create SSL directory
sudo mkdir -p /etc/openauto/ssl

# Generate private key
sudo openssl genrsa -out /etc/openauto/ssl/openauto.key 2048

# Generate certificate
sudo openssl req -new -x509 -key /etc/openauto/ssl/openauto.key \
    -out /etc/openauto/ssl/openauto.crt -days 365 \
    -subj "/C=US/ST=State/L=City/O=Organization/CN=openauto.local"

# Set permissions
sudo chown root:openauto /etc/openauto/ssl/*
sudo chmod 640 /etc/openauto/ssl/*
```

#### Enable HTTPS in Configuration
```ini
[rest_api]
enable_https = true
ssl_cert = /etc/openauto/ssl/openauto.crt
ssl_key = /etc/openauto/ssl/openauto.key
```

## Hardware-Specific Deployments

### Raspberry Pi Deployment

#### Pi-specific Configuration
```bash
# Enable GPU memory split
echo "gpu_mem=128" | sudo tee -a /boot/config.txt

# Enable hardware acceleration
echo "dtoverlay=vc4-kms-v3d" | sudo tee -a /boot/config.txt

# Disable unnecessary services
sudo systemctl disable bluetooth hciuart

# Enable hardware interfaces
sudo raspi-config nonint do_i2c 0
sudo raspi-config nonint do_spi 0
```

#### Performance Tuning
```bash
# CPU governor
echo 'performance' | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# Memory tuning
echo 'vm.swappiness=10' | sudo tee -a /etc/sysctl.conf
echo 'vm.vfs_cache_pressure=50' | sudo tee -a /etc/sysctl.conf
```

### x86/x64 Deployment

#### Hardware Acceleration
```bash
# Install VAAPI support
sudo apt install vainfo intel-media-va-driver

# Test hardware acceleration
vainfo
```

#### Audio Configuration
```bash
# Configure PulseAudio
echo 'default-sample-rate = 48000' | sudo tee -a /etc/pulse/daemon.conf
echo 'resample-method = speex-float-1' | sudo tee -a /etc/pulse/daemon.conf
```

## Monitoring and Logging

### Log Management

#### Logrotate Configuration (`/etc/logrotate.d/openauto`)
```bash
/var/log/openauto/*.log {
    daily
    missingok
    rotate 30
    compress
    delaycompress
    notifempty
    create 644 openauto openauto
    postrotate
        /bin/systemctl reload openauto 2>/dev/null || true
    endscript
}
```

#### Centralized Logging
```bash
# rsyslog configuration
echo 'local0.*    /var/log/openauto/system.log' | sudo tee -a /etc/rsyslog.d/openauto.conf
sudo systemctl restart rsyslog
```

### Monitoring

#### System Monitoring Script
```bash
#!/bin/bash
# /opt/openauto/scripts/monitor.sh

while true; do
    # Check service status
    systemctl is-active --quiet openauto || echo "$(date): OpenAuto service down" >> /var/log/openauto/monitor.log
    
    # Check memory usage
    MEM=$(ps -o pid,ppid,user,%mem,command -ax | grep autoapp | grep -v grep | awk '{print $4}')
    if (( $(echo "$MEM > 80" | bc -l) )); then
        echo "$(date): High memory usage: $MEM%" >> /var/log/openauto/monitor.log
    fi
    
    # Check REST API
    curl -f http://localhost:8080/api/v1/health > /dev/null 2>&1 || \
        echo "$(date): REST API not responding" >> /var/log/openauto/monitor.log
    
    sleep 60
done
```

### Health Checks

#### REST API Health Endpoint
```bash
# Health check script
#!/bin/bash
HEALTH=$(curl -s http://localhost:8080/api/v1/health)
STATUS=$(echo $HEALTH | jq -r '.status')

if [ "$STATUS" != "healthy" ]; then
    echo "System unhealthy: $HEALTH"
    exit 1
fi

echo "System healthy"
exit 0
```

## Backup and Recovery

### Configuration Backup
```bash
#!/bin/bash
# Backup script
BACKUP_DIR="/backup/openauto/$(date +%Y%m%d_%H%M%S)"
mkdir -p "$BACKUP_DIR"

# Backup configuration
cp -r /etc/openauto "$BACKUP_DIR/"

# Backup data
cp -r /var/lib/openauto "$BACKUP_DIR/"

# Backup service files
cp /etc/systemd/system/openauto*.service "$BACKUP_DIR/"

# Create archive
tar -czf "/backup/openauto_$(date +%Y%m%d_%H%M%S).tar.gz" "$BACKUP_DIR"
```

### Disaster Recovery
```bash
#!/bin/bash
# Recovery script
BACKUP_FILE="$1"

if [ -z "$BACKUP_FILE" ]; then
    echo "Usage: $0 <backup_file.tar.gz>"
    exit 1
fi

# Stop services
sudo systemctl stop openauto openauto-btservice

# Extract backup
tar -xzf "$BACKUP_FILE" -C /tmp/

# Restore configuration
sudo cp -r /tmp/openauto_*/etc/openauto/* /etc/openauto/

# Restore data
sudo cp -r /tmp/openauto_*/var/lib/openauto/* /var/lib/openauto/

# Restore services
sudo cp /tmp/openauto_*/openauto*.service /etc/systemd/system/

# Reload and start
sudo systemctl daemon-reload
sudo systemctl start openauto openauto-btservice
```

## Troubleshooting

### Common Deployment Issues

#### Service Won't Start
```bash
# Check service status
sudo systemctl status openauto

# Check logs
sudo journalctl -u openauto --no-pager

# Check configuration
sudo -u openauto /opt/openauto/autoapp --test-config

# Check permissions
ls -la /opt/openauto/
ls -la /etc/openauto/
```

#### Permission Issues
```bash
# Fix file permissions
sudo chown -R openauto:openauto /opt/openauto /var/lib/openauto
sudo chmod -R 755 /opt/openauto
sudo chmod -R 644 /etc/openauto/*

# Check group membership
groups openauto
```

#### Network Issues
```bash
# Check port binding
sudo netstat -tlnp | grep 8080

# Test API locally
curl http://localhost:8080/api/v1/status

# Check firewall
sudo ufw status
```

#### USB Device Issues
```bash
# Check USB devices
lsusb

# Check udev rules
sudo udevadm control --reload
sudo udevadm trigger

# Check permissions
ls -la /dev/bus/usb/*/*
```

This deployment guide provides comprehensive instructions for deploying OpenAuto in various environments with proper configuration, security, and monitoring considerations.
