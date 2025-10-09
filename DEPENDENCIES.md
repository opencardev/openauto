# OpenAuto External File Dependencies Map

## Overview
This document provides a comprehensive map of all external file dependencies, system calls, scripts, assets, and configuration files required by OpenAuto. This is essential for creating complete deb/rpm packages and understanding deployment requirements.

## Required Assets & Resources

### Embedded UI Assets (Qt Resources)
**Source**: `assets/resources.qrc` - compiled into binary
- **Icons**: 37 PNG icons for UI buttons and status indicators
  - Navigation: `aausb-hot.png`, `aawifi-hot.png`, `back-hot.png`, `home-hot.png`
  - Controls: `brightness-hot.png`, `volume-hot.png`, `power-hot.png`, `settings-hot.png`
  - Day/Night: `day-hot.png`, `night-hot.png`, `skin-hot.png`
  - Camera: `camera-hot.png`, `rearcam-hot.png`, `record-hot.png`, `recordactive-hot.png`
  - Media: `mp3-hot.png`, `play-hot.png`, `pause-hot.png`, `next-hot.png`, `prev-hot.png`
  - System: `wifi-hot.png`, `reboot-hot.png`, `sleep-hot.png`, `cursor-hot.png`
- **System**: `ico_warning.png`, `ico_info.png`, `black.png`, `coverlogo.png`
- **Font**: `Roboto-Regular.ttf` - Application font
- **Background**: `journey.jpg` - Default background image

### External Wallpaper Files (Runtime)
**Location**: Current working directory
- `wallpaper.png` - Day mode wallpaper
- `wallpaper-night.png` - Night mode wallpaper  
- `wallpaper-classic.png` - Classic day wallpaper
- `wallpaper-classic-night.png` - Classic night wallpaper
- `wallpaper-eq.png` - Equalizer background
- **Embedded Alternatives**: `wallpaper-christmas.png`, `wallpaper-firework.png` (in resources)

## Configuration Files (READ Operations)

### WiFi & Network Configuration
- **File**: `/etc/hostapd/hostapd.conf`
- **Parameters Read**:
  - `ssid` - WiFi network name
  - `wpa_passphrase` - WiFi password
- **Used by**: 
  - `src/btservice/AndroidBluetoothServer.cpp`
  - `src/autoapp/Service/WifiProjection/WifiProjectionService.cpp`
- **Purpose**: WiFi hotspot configuration for wireless Android Auto

### System Configuration Detection
- **File**: `/etc/crankshaft.branch`
- **Used by**: `src/autoapp/UI/MainWindow.cpp`
- **Purpose**: Detect Crankshaft-NG installation

### Application Configuration
- **File**: `openauto-logs.ini`
- **Used by**: `src/autoapp/autoapp.cpp`
- **Purpose**: Logging configuration settings
- **Method**: `std::ifstream`

- **File**: `openauto_wifi_recent.ini`
- **Used by**: `src/autoapp/Configuration/RecentAddressesList.cpp`
- **Purpose**: Recently used WiFi connections
- **Access**: Read/Write operations

### Audio Configuration
- **MP3 Configuration**: Via IConfiguration interface
  - `General.Mp3MasterPath` - Music directory path
  - `General.Mp3SubFolder` - Subfolder selection
  - `General.Mp3Track` - Current track
  - `General.Mp3AutoPlay` - Auto-play setting
- **Default Music Path**: `/media/CSSTORAGE/Music`

## System Scripts & External Commands

### 1. Crankshaft Day/Night Service
- **Script**: `/opt/crankshaft/service_daynight.sh`
- **Used by**: `src/autoapp/autoapp.cpp`
- **Parameters**:
  - `app night` - Switch to night mode
  - `app day` - Switch to day mode
- **Purpose**: System-wide day/night mode switching
- **Triggers**: UI day/night buttons

### 2. Crankshaft Camera Control
- **Script**: `/opt/crankshaft/cameracontrol.py`
- **Used by**: `src/autoapp/autoapp.cpp`
- **Parameters**: 9 camera control commands
  - `Background` / `Foreground` - Camera display mode
  - `PosYUp` / `PosYDown` - Camera position control
  - `ZoomPlus` / `ZoomMinus` - Camera zoom control
  - `Record` / `Stop` / `Save` - Recording functions
- **Purpose**: Rear-view camera control system
- **Execution**: Background processes

### 3. AutoApp Helper Utilities
- **Script**: `/usr/local/bin/autoapp_helper`
- **Used by**: 
  - `src/autoapp/UI/ConnectDialog.cpp` - Parameter: `updaterecent`
  - `src/autoapp/autoapp.cpp` - Parameter: `usbreset`
- **Purpose**: WiFi connection management and USB reset

### 4. Crankshaft System Management
- **Script**: `/usr/local/bin/crankshaft`
- **Used by**: `src/autoapp/UI/SettingsWindow.cpp`, `src/autoapp/UI/UpdateDialog.cpp`
- **Commands**:
  - `update check` / `update cancel` - Update management
  - `update csmt` / `update udev` / `update openauto` / `update system` - Component updates
  - `rtc sync` - RTC synchronization
  - `debuglog` - Debug log generation
  - `network auto` - Network configuration
  - `samba start` / `samba stop` - SMB service control

## Temporary Files & System Signals

### Application Control Files
- `/tmp/entityexit` - Entity exit flag (Read/Write)
- `/tmp/shutdown` - System shutdown signal (Write)
- `/tmp/reboot` - System reboot signal (Write)

### Media & Storage
- `/media/CSSTORAGE/Music` - Default music directory
- Various sensor files (paths determined at runtime)

## UI & Display System Dependencies

### Brightness Control System
**Configuration Variables** (read from Crankshaft config):
- `BR_MIN`, `BR_MAX`, `BR_STEP` - Brightness range and step
- `BR_DAY`, `BR_NIGHT` - Day/night brightness levels
- `DISP_BRIGHTNESS_1` through `DISP_BRIGHTNESS_5` - 5-point brightness curve
- `LUX_LEVEL_1` through `LUX_LEVEL_5` - Light sensor thresholds
- `TSL2561_CHECK_INTERVAL` - Light sensor polling interval
- `TSL2561_DAYNIGHT_ON_STEP` - Day/night transition step

### Time & RTC Configuration
- `RTC_DAY_START`, `RTC_NIGHT_START` - Day/night schedule
- `DISCONNECTION_POWEROFF_MINS` - Shutdown timer
- `DISCONNECTION_SCREEN_POWEROFF_SECS` - Screen timeout

### Audio System Configuration
- System volume control via UI sliders
- Audio capture volume: `/boot/crankshaft/capvolume`

### GPIO & Hardware
- `ENABLE_GPIO` - GPIO control enable/disable

## Network Interface Dependencies

### Required Network Interfaces
- **wlan0** - WiFi interface for hardware MAC address retrieval
- **Bluetooth** - Local Bluetooth device for pairing/connection

## Packaging Requirements for deb/rpm

### Essential Files to Include
```
/opt/crankshaft/
├── cameracontrol.py          # Camera control script
└── service_daynight.sh       # Day/night mode script

/usr/local/bin/
├── autoapp_helper            # Connection helper utility  
└── crankshaft               # System management script

/etc/
└── hostapd/
    └── hostapd.conf         # WiFi configuration (template)

Application Directory/
├── wallpaper.png            # Optional day wallpaper
├── wallpaper-night.png      # Optional night wallpaper
├── wallpaper-classic.png    # Optional classic day
├── wallpaper-classic-night.png # Optional classic night
├── wallpaper-eq.png         # Optional EQ background
├── openauto-logs.ini        # Logging configuration
└── openauto_wifi_recent.ini # WiFi history (created at runtime)
```

### Directories to Create
```
/tmp/                        # For application control files
/media/CSSTORAGE/Music/      # Default music directory
/boot/crankshaft/            # For capvolume configuration
```

### System Integration Requirements
1. **Systemd Services**: Integration with system day/night services
2. **Network Management**: WiFi hotspot configuration
3. **Audio System**: ALSA/PulseAudio integration
4. **Camera System**: V4L2 camera device access
5. **USB Management**: USB device reset capabilities
6. **Bluetooth**: BlueZ integration for device pairing

### Security Considerations for Packaging
1. **Privileged Operations**: Scripts require appropriate sudo permissions
2. **File Permissions**: Camera and system scripts need execution permissions
3. **Network Access**: WiFi and Bluetooth configuration access
4. **System Control**: Shutdown/reboot signal creation
5. **Hardware Access**: Camera, audio, and GPIO device access

### Runtime Dependencies
- **Qt5 Multimedia**: Media playback and UI
- **BlueZ**: Bluetooth functionality
- **NetworkManager/hostapd**: WiFi management  
- **V4L2**: Camera system
- **ALSA/PulseAudio**: Audio system
- **Python3**: Camera control scripts
- **Bash**: System management scripts

### Post-Installation Requirements
1. Configure hostapd for WiFi hotspot
2. Set up proper permissions for system scripts
3. Create required directories with appropriate ownership
4. Configure systemd services for integration
5. Set up audio group permissions
6. Configure camera device permissions

---

**Last Updated**: October 8, 2025  
**Analysis Scope**: Complete OpenAuto source code with deep UI and configuration analysis  
**Purpose**: Complete packaging requirements for deb/rpm distribution

## Build System Dependencies

### Standard Library Paths
- **Include Paths**:
  - `/usr/include` - System headers
  - `/usr/local/include` - Local installation headers
  - `/opt/local/include` - Optional package headers
- **Library Paths**:
  - `/usr/lib` - System libraries
  - `/usr/local/lib` - Local installation libraries
  - `/opt/local/lib` - Optional package libraries

### Package Management (Build Environment)
- **APT Cache**: `/var/cache/apt/archives/` (in build scripts)
- **Temporary Build**: `/tmp/armhf/`, `/tmp/amd64/` (for cross-compilation)

## Network Interface Dependencies

### WiFi Interface
- **Interface**: `wlan0`
- **Used by**: `src/autoapp/Service/WifiProjection/WifiProjectionService.cpp`
- **Purpose**: Get hardware MAC address for WiFi projection
- **Method**: `QNetworkInterface::interfaceFromName("wlan0").hardwareAddress()`

## Security Considerations

### High-Risk Operations
1. **System Command Execution**: Multiple `system()` calls executing external scripts
2. **File System Access**: Reading system configuration files
3. **Network Interface Access**: Direct hardware address retrieval
4. **Temporary File Creation**: Creating system control files in `/tmp/`

### Privilege Requirements
- **Root/Sudo Access**: Likely required for:
  - Reading `/etc/hostapd/hostapd.conf`
  - Executing camera control scripts in `/opt/crankshaft/`
  - System shutdown/reboot operations
- **Network Permissions**: Required for WiFi interface access

## Deployment Requirements

### Required External Components
1. **Crankshaft NG Framework**: 
   - Camera control scripts (`/opt/crankshaft/cameracontrol.py`)
   - Update management system (`crankshaft update`)
2. **AutoApp Helper**: `/usr/local/bin/autoapp_helper`
3. **Hostapd Configuration**: `/etc/hostapd/hostapd.conf`
4. **Network Interface**: `wlan0` configured and available

### Required System Directories
- `/opt/crankshaft/` - Crankshaft scripts location
- `/usr/local/bin/` - AutoApp helper location
- `/etc/hostapd/` - WiFi configuration
- `/tmp/` - Temporary file operations

## Runtime Dependencies

### CSNG Integration Points
- **Exit to CSNG**: Referenced in `src/autoapp/App.cpp` (line 131)
- **Update System**: Integration with Crankshaft update mechanism
- **Camera Control**: Deep integration with Crankshaft camera management

### File Access Patterns
1. **Configuration Reading**: At startup and during WiFi setup
2. **Temporary File Creation**: During operation state changes
3. **Script Execution**: Event-driven (user interactions, camera controls)
4. **System Control**: Shutdown/reboot operations

## Recommendations

### For Standalone Deployment
1. **Mock External Scripts**: Create stub implementations for non-Crankshaft environments
2. **Configuration Alternatives**: Provide alternative configuration sources
3. **Graceful Degradation**: Handle missing external dependencies gracefully
4. **Security Hardening**: Validate all external script paths and parameters

### For Security Auditing
1. **Input Validation**: All external file paths and script parameters should be validated
2. **Privilege Separation**: Consider running with minimal required privileges
3. **Sandbox Execution**: External script execution should be sandboxed
4. **File Permission Checks**: Verify file permissions before access

---

**Last Updated**: October 8, 2025  
**Analysis Scope**: OpenAuto source code in `/home/pi/openauto_ori/`  
**Purpose**: Deployment planning, security analysis, and system integration