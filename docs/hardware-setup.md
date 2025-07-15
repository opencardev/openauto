# Hardware Setup Guide

This document provides comprehensive guidance for setting up OpenAuto on various hardware platforms, with focus on Raspberry Pi and automotive environments.

## Table of Contents
- [Overview](#overview)
- [Raspberry Pi Setup](#raspberry-pi-setup)
- [x86 Hardware Setup](#x86-hardware-setup)
- [Automotive Integration](#automotive-integration)
- [Display Configuration](#display-configuration)
- [Audio Configuration](#audio-configuration)
- [Input Device Setup](#input-device-setup)
- [Performance Optimization](#performance-optimization)
- [Troubleshooting](#troubleshooting)

## Overview

OpenAuto supports various hardware configurations for automotive and desktop environments. This guide covers:
- Raspberry Pi (recommended for automotive use)
- x86/x64 systems (desktop and industrial)
- Hardware-specific optimizations
- Performance tuning guidelines
- Common troubleshooting scenarios

### Supported Hardware Platforms

| Platform | Architecture | RAM | Storage | Status |
|----------|-------------|-----|---------|--------|
| Raspberry Pi 4B | ARM64 | 4GB+ | 32GB+ | ✅ Recommended |
| Raspberry Pi 3B+ | ARM64 | 1GB | 16GB+ | ⚠️ Limited |
| Intel NUC | x86_64 | 8GB+ | 128GB+ | ✅ Excellent |
| Generic x86 | x86_64 | 4GB+ | 64GB+ | ✅ Good |
| Automotive ECU | ARM/x86 | 2GB+ | 16GB+ | ⚠️ Custom |

## Raspberry Pi Setup

### Hardware Requirements

#### Recommended Configuration
- **Raspberry Pi 4B**: 4GB or 8GB RAM
- **MicroSD Card**: 32GB Class 10 or higher (SanDisk Ultra recommended)
- **Power Supply**: Official Raspberry Pi 4 power supply (5V 3A)
- **Display**: 7" or larger touchscreen
- **USB Hub**: Powered USB 3.0 hub for multiple devices
- **Cooling**: Active cooling (fan + heatsinks)

#### Optional Components
- **USB-C to USB-A Adapter**: For Android device connection
- **HDMI Cable**: For external display connection
- **Audio HAT**: For improved audio quality
- **GPIO Breakout**: For integration with vehicle systems
- **Camera Module**: For backup camera functionality

### Raspberry Pi OS Installation

#### Download and Flash OS
```bash
# Download Raspberry Pi Imager
# From: https://www.raspberrypi.org/software/

# Flash Raspberry Pi OS Lite (64-bit) to SD card
# Enable SSH and Wi-Fi in imager settings
```

#### Initial Configuration
```bash
# SSH into Raspberry Pi
ssh pi@raspberrypi.local

# Update system
sudo apt update && sudo apt upgrade -y

# Install essential packages
sudo apt install -y \
    git \
    cmake \
    build-essential \
    pkg-config \
    vim \
    htop \
    tree
```

#### Enable Required Interfaces
```bash
# Open raspi-config
sudo raspi-config

# Enable:
# - SSH (Interface Options -> SSH)
# - I2C (Interface Options -> I2C)
# - SPI (Interface Options -> SPI)
# - Camera (Interface Options -> Camera)
# - GL Driver (Advanced Options -> GL Driver -> GL (Fake KMS))
```

#### Configure Boot Options
```bash
# Edit boot configuration
sudo vim /boot/config.txt

# Add/modify these lines:
# Enable 64-bit kernel
arm_64bit=1

# GPU memory split (for video processing)
gpu_mem=128

# Enable HDMI audio
hdmi_drive=2

# Force HDMI output
hdmi_force_hotplug=1

# Set HDMI resolution (adjust as needed)
hdmi_group=2
hdmi_mode=82

# Enable hardware acceleration
dtoverlay=vc4-kms-v3d
max_framebuffers=2

# Disable rainbow splash
disable_splash=1

# Reboot to apply changes
sudo reboot
```

### Performance Optimization

#### CPU Governor
```bash
# Set performance governor
echo 'performance' | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# Make permanent
echo 'GOVERNOR="performance"' | sudo tee -a /etc/default/cpufrequtils
```

#### Memory Configuration
```bash
# Edit system configuration
sudo vim /boot/cmdline.txt

# Add to existing line:
cma=64M gpu_mem=128
```

#### Swap Configuration
```bash
# Disable swap for better performance
sudo dphys-swapfile swapoff
sudo dphys-swapfile uninstall
sudo update-rc.d dphys-swapfile remove
```

### Cooling Setup

#### Active Cooling Configuration
```bash
# Install fan control
sudo apt install -y rpi.gpio-common

# Create fan control script
sudo vim /usr/local/bin/fan_control.py
```

```python
#!/usr/bin/env python3
import RPi.GPIO as GPIO
import time
import subprocess

FAN_PIN = 18
TEMP_THRESHOLD = 65

GPIO.setmode(GPIO.BCM)
GPIO.setup(FAN_PIN, GPIO.OUT)
pwm = GPIO.PWM(FAN_PIN, 1000)
pwm.start(0)

try:
    while True:
        temp = float(subprocess.check_output(['vcgencmd', 'measure_temp']).decode().split('=')[1].split("'")[0])
        
        if temp > TEMP_THRESHOLD:
            duty_cycle = min(100, (temp - TEMP_THRESHOLD) * 10)
            pwm.ChangeDutyCycle(duty_cycle)
        else:
            pwm.ChangeDutyCycle(0)
        
        time.sleep(5)
except KeyboardInterrupt:
    GPIO.cleanup()
```

```bash
# Make executable
sudo chmod +x /usr/local/bin/fan_control.py

# Create systemd service
sudo vim /etc/systemd/system/fan-control.service
```

```ini
[Unit]
Description=Fan Control Service
After=multi-user.target

[Service]
Type=simple
ExecStart=/usr/local/bin/fan_control.py
Restart=always
User=root

[Install]
WantedBy=multi-user.target
```

```bash
# Enable and start service
sudo systemctl enable fan-control.service
sudo systemctl start fan-control.service
```

## x86 Hardware Setup

### Minimum Requirements
- **CPU**: Intel Core i3 or AMD Ryzen 3 (or equivalent)
- **RAM**: 4GB (8GB recommended)
- **Storage**: 64GB SSD
- **Graphics**: Intel HD Graphics or dedicated GPU
- **USB**: USB 3.0 ports for Android device connection

### BIOS/UEFI Configuration
```
# Boot Settings
- Fast Boot: Disabled
- Secure Boot: Disabled (for Linux)
- Legacy Support: Enabled

# Power Management
- Wake on USB: Enabled
- Power on after power loss: Enabled

# Integrated Peripherals
- USB 3.0: Enabled
- Audio: Enabled
- Network: Enabled
```

### Ubuntu Installation
```bash
# Download Ubuntu 22.04 LTS (Desktop or Server)
# Flash to USB drive using Rufus (Windows) or dd (Linux)

# Install with following partitions:
# /boot/efi - 512MB EFI System Partition
# / - 32GB ext4 (root)
# /home - remaining space ext4 (user data)

# Post-installation updates
sudo apt update && sudo apt upgrade -y

# Install essential packages
sudo apt install -y \
    ubuntu-restricted-extras \
    build-essential \
    cmake \
    git \
    pkg-config \
    curl \
    wget
```

## Automotive Integration

### Vehicle Power Management

#### 12V Power Supply
```bash
# Install automotive power management
sudo apt install -y rpi-gpio-common

# Create power management script
sudo vim /usr/local/bin/power_manager.py
```

```python
#!/usr/bin/env python3
import RPi.GPIO as GPIO
import subprocess
import time
import signal
import sys

# GPIO pins
IGNITION_PIN = 23
POWER_HOLD_PIN = 24

# Timing
SHUTDOWN_DELAY = 30  # seconds

GPIO.setmode(GPIO.BCM)
GPIO.setup(IGNITION_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(POWER_HOLD_PIN, GPIO.OUT)
GPIO.output(POWER_HOLD_PIN, GPIO.HIGH)

def shutdown_handler(signum, frame):
    print("Initiating safe shutdown...")
    GPIO.output(POWER_HOLD_PIN, GPIO.LOW)
    subprocess.run(['sudo', 'shutdown', '-h', 'now'])
    sys.exit(0)

signal.signal(signal.SIGINT, shutdown_handler)
signal.signal(signal.SIGTERM, shutdown_handler)

def ignition_callback(channel):
    if GPIO.input(IGNITION_PIN) == GPIO.LOW:
        print("Ignition off detected, starting shutdown timer...")
        time.sleep(SHUTDOWN_DELAY)
        if GPIO.input(IGNITION_PIN) == GPIO.LOW:
            shutdown_handler(None, None)

GPIO.add_event_detect(IGNITION_PIN, GPIO.FALLING, callback=ignition_callback, bouncetime=1000)

try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    GPIO.cleanup()
```

#### CAN Bus Integration (Optional)
```bash
# Install CAN utilities
sudo apt install -y can-utils

# Configure CAN interface
sudo vim /etc/systemd/network/can0.network
```

```ini
[Match]
Name=can0

[CAN]
BitRate=500000
RestartSec=100ms
```

### Audio Integration

#### Configure Audio Output
```bash
# List audio devices
aplay -l

# Set default audio device
sudo vim /etc/asound.conf
```

```
pcm.!default {
    type hw
    card 1
    device 0
}

ctl.!default {
    type hw
    card 1
}
```

#### Steering Wheel Controls
```bash
# Install input utilities
sudo apt install -y input-utils

# Map steering wheel buttons
sudo vim /etc/udev/rules.d/99-steering-wheel.rules
```

```
# Steering wheel control mapping
SUBSYSTEM=="input", ATTRS{name}=="Steering Wheel Controls", MODE="0666", GROUP="input"
```

## Display Configuration

### Touchscreen Setup

#### Official Raspberry Pi Touchscreen
```bash
# Enable I2C
sudo raspi-config
# Interface Options -> I2C -> Enable

# Install touchscreen drivers
sudo apt install -y raspberrypi-ui-mods

# Configure display rotation
sudo vim /boot/config.txt
# Add: display_rotate=2  # for 180-degree rotation
```

#### Generic HDMI Touchscreen
```bash
# Install xinput for calibration
sudo apt install -y xinput-calibrator

# Run calibration
DISPLAY=:0 xinput_calibrator

# Save calibration to file
sudo vim /usr/share/X11/xorg.conf.d/99-calibration.conf
```

### Resolution and Scaling
```bash
# Set display resolution
xrandr --output HDMI-1 --mode 1920x1080 --rate 60

# Configure scaling for high DPI
export GDK_SCALE=1.5
export QT_SCALE_FACTOR=1.5

# Make permanent in ~/.bashrc
echo 'export GDK_SCALE=1.5' >> ~/.bashrc
echo 'export QT_SCALE_FACTOR=1.5' >> ~/.bashrc
```

## Audio Configuration

### System Audio Setup
```bash
# Install audio packages
sudo apt install -y \
    alsa-utils \
    pulseaudio \
    pulseaudio-module-bluetooth \
    pavucontrol

# Configure PulseAudio
systemctl --user enable pulseaudio
systemctl --user start pulseaudio
```

### USB Audio Devices
```bash
# List USB audio devices
lsusb | grep -i audio

# Test audio output
speaker-test -D plughw:1,0 -c 2 -t sine -f 1000

# Configure as default device
sudo vim ~/.asoundrc
```

```
pcm.!default {
    type hw
    card 1
    device 0
}

ctl.!default {
    type hw
    card 1
}
```

### Bluetooth Audio
```bash
# Install Bluetooth packages
sudo apt install -y \
    bluetooth \
    bluez \
    bluez-tools \
    pulseaudio-module-bluetooth

# Enable and start Bluetooth
sudo systemctl enable bluetooth
sudo systemctl start bluetooth

# Configure auto-connect
sudo vim /etc/bluetooth/main.conf
# Uncomment: AutoEnable=true
```

## Input Device Setup

### USB Input Devices
```bash
# Install input utilities
sudo apt install -y \
    evtest \
    input-utils \
    joystick

# Test input devices
sudo evtest

# List input devices
ls /dev/input/
```

### Custom Input Mapping
```bash
# Create input mapping
sudo vim /etc/udev/rules.d/99-openauto-input.rules
```

```
# Android Auto input device
SUBSYSTEM=="input", ATTRS{idVendor}=="18d1", ATTRS{idProduct}=="4ee7", MODE="0666", GROUP="input"

# Custom steering wheel controls
SUBSYSTEM=="input", ATTRS{name}=="*Steering*", MODE="0666", GROUP="input"
```

## Performance Optimization

### System Optimization
```bash
# Disable unnecessary services
sudo systemctl disable \
    bluetooth \
    cups \
    avahi-daemon \
    triggerhappy

# Optimize kernel parameters
sudo vim /etc/sysctl.conf
```

```
# Network optimizations
net.core.rmem_max = 16777216
net.core.wmem_max = 16777216
net.ipv4.tcp_rmem = 4096 87380 16777216
net.ipv4.tcp_wmem = 4096 65536 16777216

# Memory optimizations
vm.swappiness = 1
vm.vfs_cache_pressure = 50
```

### USB Optimization
```bash
# Increase USB buffer sizes
echo 'SUBSYSTEM=="usb", ACTION=="add", ATTR{bConfigurationValue}="1"' | sudo tee -a /etc/udev/rules.d/99-usb-optimization.rules

# USB autosuspend disable for Android devices
echo 'SUBSYSTEM=="usb", ATTRS{idVendor}=="18d1", ATTR{power/autosuspend}="-1"' | sudo tee -a /etc/udev/rules.d/99-usb-optimization.rules
```

### Graphics Optimization
```bash
# Configure GPU memory split
sudo vim /boot/config.txt
# Add: gpu_mem=128

# Enable hardware acceleration
echo 'export QT_QPA_EGLFS_FORCE888=1' >> ~/.bashrc
echo 'export QT_QPA_EGLFS_HIDECURSOR=1' >> ~/.bashrc
```

## Troubleshooting

### Common Hardware Issues

#### USB Connection Problems
```bash
# Check USB device detection
lsusb

# Monitor USB events
sudo udevadm monitor --environment --udev

# Check USB power management
cat /sys/bus/usb/devices/*/power/autosuspend

# Disable USB autosuspend globally
echo 'SUBSYSTEM=="usb", ATTR{power/autosuspend}="-1"' | sudo tee /etc/udev/rules.d/99-usb-autosuspend.rules
```

#### Audio Issues
```bash
# Check audio devices
aplay -l
arecord -l

# Test audio output
speaker-test -t sine -f 1000 -c 2

# Check PulseAudio status
pulseaudio --check -v
systemctl --user status pulseaudio
```

#### Display Issues
```bash
# Check display configuration
xrandr
fbset

# Test HDMI connection
tvservice -s
vcgencmd display_power

# Force HDMI mode
sudo vim /boot/config.txt
# Add: hdmi_force_hotplug=1
```

#### Temperature Monitoring
```bash
# Check CPU temperature
vcgencmd measure_temp

# Monitor system temperatures
sensors

# Check for thermal throttling
vcgencmd get_throttled
```

### Performance Issues

#### CPU Performance
```bash
# Check CPU frequency
cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_cur_freq

# Set performance governor
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
```

#### Memory Usage
```bash
# Check memory usage
free -h
cat /proc/meminfo

# Check for memory leaks
ps aux --sort=-%mem | head
```

#### Storage Performance
```bash
# Test SD card performance
sudo hdparm -tT /dev/mmcblk0

# Check filesystem errors
sudo fsck -f /dev/mmcblk0p2
```

### Network Connectivity
```bash
# Check network interfaces
ip addr show

# Test connectivity
ping -c 4 8.8.8.8

# Check DNS resolution
nslookup google.com
```

## Hardware Monitoring

### System Monitoring Script
```bash
# Create monitoring script
sudo vim /usr/local/bin/hardware_monitor.sh
```

```bash
#!/bin/bash

LOG_FILE="/var/log/hardware_monitor.log"

while true; do
    echo "$(date): Hardware Status Check" >> $LOG_FILE
    
    # Temperature
    TEMP=$(vcgencmd measure_temp | cut -d= -f2 | cut -d\' -f1)
    echo "CPU Temperature: ${TEMP}°C" >> $LOG_FILE
    
    # Memory usage
    MEM=$(free | grep Mem | awk '{printf "%.1f", $3/$2 * 100.0}')
    echo "Memory Usage: ${MEM}%" >> $LOG_FILE
    
    # CPU usage
    CPU=$(top -bn1 | grep "Cpu(s)" | awk '{print $2}' | cut -d% -f1)
    echo "CPU Usage: ${CPU}%" >> $LOG_FILE
    
    # Storage usage
    DISK=$(df -h / | awk 'NR==2{printf "%s", $5}')
    echo "Disk Usage: ${DISK}" >> $LOG_FILE
    
    echo "---" >> $LOG_FILE
    sleep 300  # Check every 5 minutes
done
```

```bash
# Make executable and create service
sudo chmod +x /usr/local/bin/hardware_monitor.sh

sudo vim /etc/systemd/system/hardware-monitor.service
```

```ini
[Unit]
Description=Hardware Monitoring Service
After=multi-user.target

[Service]
Type=simple
ExecStart=/usr/local/bin/hardware_monitor.sh
Restart=always
User=root

[Install]
WantedBy=multi-user.target
```

```bash
# Enable and start monitoring
sudo systemctl enable hardware-monitor.service
sudo systemctl start hardware-monitor.service
```

## Best Practices

### Hardware Selection
1. Choose quality components for automotive environment
2. Ensure adequate cooling for enclosed installations
3. Use automotive-grade power supplies
4. Select displays with wide viewing angles
5. Consider vibration and temperature resistance

### Installation Guidelines
1. Secure all connections and components
2. Provide adequate ventilation
3. Route cables away from heat sources
4. Use proper grounding techniques
5. Test thoroughly before final installation

### Maintenance
1. Regular temperature monitoring
2. Periodic cleaning of cooling components
3. SD card health checks (Raspberry Pi)
4. Software updates and security patches
5. Backup configuration files regularly

## Related Documentation

- [Build Guide](build-guide.md) - Software build instructions
- [Deployment Guide](deployment-guide.md) - Software deployment
- [Performance Guide](performance-guide.md) - Performance optimization
- [Troubleshooting Guide](troubleshooting-guide.md) - General troubleshooting
