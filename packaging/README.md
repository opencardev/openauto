# OpenAuto Debian Package

## Package Details

- **Package Name**: `openauto-modern`
- **Architecture**: `armhf` (Raspberry Pi) / `amd64` (x86_64)
- **Section**: `electronics`
- **Priority**: `optional`

## Installation

### From Source
```bash
# Build and install package
./build-package.sh
sudo apt install ./build-package/openauto-modern*.deb
```

### Package Contents

#### Binaries
- `/opt/openauto/autoapp` - Main OpenAuto application
- `/opt/openauto/btservice` - Bluetooth service
- `/opt/openauto/scripts/openauto-setup.sh` - Setup script
- `/opt/openauto/scripts/openauto-monitor.sh` - Monitoring script

#### Configuration Files
- `/etc/openauto/openauto.conf` - Main configuration
- `/etc/openauto/logger.conf` - Logging configuration  
- `/etc/openauto/services.conf` - Services configuration

#### System Integration
- `/etc/systemd/system/openauto.service` - Main service
- `/etc/systemd/system/openauto-btservice.service` - Bluetooth service
- `/etc/udev/rules.d/99-openauto.rules` - USB device rules
- `/etc/logrotate.d/openauto` - Log rotation

#### Data Directories
- `/var/lib/openauto/` - Application data
- `/var/log/openauto/` - Log files

#### Documentation
- `/usr/share/doc/openauto-modern/` - Documentation

## Service Management

### Service Status
```bash
# Check all services
sudo systemctl status openauto openauto-btservice

# Start services
sudo systemctl start openauto openauto-btservice

# Stop services  
sudo systemctl stop openauto openauto-btservice

# Restart services
sudo systemctl restart openauto openauto-btservice

# Enable auto-start
sudo systemctl enable openauto openauto-btservice

# Disable auto-start
sudo systemctl disable openauto openauto-btservice
```

### Logs
```bash
# View service logs
sudo journalctl -u openauto -f
sudo journalctl -u openauto-btservice -f

# View application logs
tail -f /var/log/openauto/openauto.log

# View all OpenAuto logs
sudo journalctl -u openauto* -f
```

## Configuration

### Main Configuration (`/etc/openauto/openauto.conf`)
```ini
[system]
log_level = INFO
data_directory = /var/lib/openauto
temp_directory = /tmp/openauto
user = openauto
group = openauto

[android_auto]
usb_hotplug = true
wifi_projection = true
audio_output = pulse
video_output = drm

[modern_api]
enable_rest_api = true
rest_api_port = 8080
rest_api_bind = 0.0.0.0
enable_event_bus = true
enable_state_machine = true

[logging]
async_logging = true
max_queue_size = 10000
console_output = true
file_output = true
file_path = /var/log/openauto/openauto.log
file_max_size = 50MB
file_rotation_count = 5
json_format = false
```

### Logger Configuration (`/etc/openauto/logger.conf`)
```ini
[levels]
global = INFO
system = DEBUG
android_auto = INFO
ui = WARN

[sinks]
console = true
file = true
syslog = true

[formatters]
console_format = console
file_format = detailed
```

## REST API

### Endpoints
- `GET /api/v1/health` - Health check
- `GET /api/v1/status` - System status
- `GET /api/v1/config` - Configuration
- `PUT /api/v1/config` - Update configuration

### Example Usage
```bash
# Health check
curl http://localhost:8080/api/v1/health

# Get status
curl http://localhost:8080/api/v1/status

# Get configuration
curl http://localhost:8080/api/v1/config
```

## Troubleshooting

### Service Issues
```bash
# Service won't start
sudo systemctl status openauto
sudo journalctl -u openauto --no-pager

# Permission issues
sudo ls -la /opt/openauto/
sudo ls -la /var/lib/openauto/
sudo ls -la /var/log/openauto/

# Reset permissions
sudo /opt/openauto/scripts/openauto-setup.sh
```

### USB Device Issues
```bash
# Check USB devices
lsusb

# Check udev rules
sudo udevadm control --reload-rules
sudo udevadm trigger

# Test device permissions
ls -la /dev/bus/usb/*/*
```

### Network Issues
```bash
# Check API port
sudo netstat -tlnp | grep 8080

# Test API locally
curl http://localhost:8080/api/v1/health

# Check firewall
sudo ufw status
```

## Uninstallation

### Remove Package
```bash
# Remove package but keep configuration
sudo apt remove openauto-modern

# Remove package and all data (purge)
sudo apt purge openauto-modern

# Check what remains
dpkg -l | grep openauto
```

### Manual Cleanup (if needed)
```bash
# Stop services
sudo systemctl stop openauto openauto-btservice
sudo systemctl disable openauto openauto-btservice

# Remove files
sudo rm -rf /opt/openauto
sudo rm -rf /etc/openauto
sudo rm -rf /var/lib/openauto
sudo rm -rf /var/log/openauto
sudo rm -f /etc/systemd/system/openauto*.service
sudo rm -f /etc/udev/rules.d/99-openauto.rules
sudo rm -f /etc/logrotate.d/openauto

# Remove user
sudo userdel openauto
sudo groupdel openauto

# Reload systemd
sudo systemctl daemon-reload
sudo udevadm control --reload-rules
```

## Package Development

### Build Dependencies
```bash
sudo apt install \
    build-essential cmake pkg-config git \
    libboost-all-dev libprotobuf-dev protobuf-compiler \
    libssl-dev libusb-1.0-0-dev libudev-dev \
    qtbase5-dev qtmultimedia5-dev qtconnectivity5-dev \
    libasound2-dev libpulse-dev librtaudio-dev \
    nlohmann-json3-dev libevent-dev
```

### Build Process
```bash
# Configure
cmake -DCMAKE_BUILD_TYPE=Release \
      -DENABLE_MODERN_API=ON \
      -DENABLE_REST_API=ON \
      -DENABLE_EVENT_BUS=ON \
      -DENABLE_STATE_MACHINE=ON \
      .

# Build
make -j$(nproc)

# Create package
cpack -G DEB
```

### Package Testing
```bash
# Install in test environment
sudo dpkg -i openauto-modern*.deb

# Test functionality
sudo systemctl status openauto
curl http://localhost:8080/api/v1/health

# Remove
sudo apt purge openauto-modern
```
