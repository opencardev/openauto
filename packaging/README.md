# OpenAuto Packaging

This directory contains packaging resources for OpenAuto debian packages.

## Structure

```
packaging/
├── debian/          # Debian package maintenance scripts
│   ├── postinst    # Post-installation script
│   ├── prerm       # Pre-removal script
│   ├── postrm      # Post-removal script
│   └── preinst     # Pre-installation script
├── systemd/         # Systemd service files
│   ├── openauto.service           # Main OpenAuto service
│   └── openauto-btservice.service # Bluetooth service
└── udev/            # Udev rules
    └── 99-openauto.rules          # Android device USB access rules
```

## Debian Scripts

### postinst
Executed after package installation. Performs:
- Reloads udev rules to apply Android device permissions
- Reloads systemd daemon
- Enables openauto and openauto-btservice systemd services
- Does not auto-start services (user can start manually)

### prerm
Executed before package removal. Performs:
- Stops openauto and openauto-btservice systemd services

### postrm
Executed after package removal. Performs:
- On purge: disables systemd services and reloads daemon

### preinst
Executed before package installation. Currently a placeholder.

## Systemd Services

### openauto.service
Main service that runs the `autoapp` binary. Configured to:
- Start after network, sound, and bluetooth targets
- Run as root with audio, video, and plugdev groups
- Auto-restart on failure with 5-second delay
- Use eglfs QT platform for direct framebuffer rendering

### openauto-btservice.service
Bluetooth pairing service that runs the `btservice` binary. Configured to:
- Start after bluetooth target
- Run as root
- Auto-restart on failure with 5-second delay

## Udev Rules

### 99-openauto.rules
Grants USB access permissions for Android devices using Android Auto. Includes:
- Common Android device vendor IDs (Google, Samsung, LG, etc.)
- Android Open Accessory (AOA) protocol devices
- Permissions: MODE="0666", GROUP="plugdev", TAG+="uaccess"

The high number (99) ensures these rules run after system default rules.

## Building Packages

The packaging files are automatically included when building debian packages with CPack:

```bash
mkdir -p build-release
cd build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
make
cpack
```

This will create a `.deb` package that includes:
- Binaries in `/usr/bin/`
- Udev rules in `/lib/udev/rules.d/`
- Systemd services in `/lib/systemd/system/`
- Automatic service enablement on installation

## Manual Installation

If not using the debian package, you can manually install:

```bash
# Install udev rules
sudo cp packaging/udev/99-openauto.rules /lib/udev/rules.d/
sudo udevadm control --reload-rules
sudo udevadm trigger

# Install systemd services
sudo cp packaging/systemd/*.service /lib/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable openauto.service
sudo systemctl enable openauto-btservice.service

# Start services
sudo systemctl start openauto
sudo systemctl start openauto-btservice
```

## Testing

After installation, verify:

```bash
# Check udev rules are loaded
ls -l /lib/udev/rules.d/99-openauto.rules

# Check services are enabled
systemctl is-enabled openauto.service
systemctl is-enabled openauto-btservice.service

# Check services status
systemctl status openauto
systemctl status openauto-btservice

# View service logs
journalctl -u openauto -f
journalctl -u openauto-btservice -f
```

## Troubleshooting

### USB device not detected
1. Check udev rules are installed: `ls /lib/udev/rules.d/99-openauto.rules`
2. Reload udev: `sudo udevadm control --reload-rules && sudo udevadm trigger`
3. Check device vendor ID: `lsusb` and add to rules if needed

### Service not starting
1. Check service status: `systemctl status openauto`
2. View logs: `journalctl -u openauto -n 50`
3. Check binary exists: `which autoapp`
4. Verify dependencies are installed

### Bluetooth not pairing
1. Check bluetooth service: `systemctl status bluetooth`
2. Check btservice: `systemctl status openauto-btservice`
3. View logs: `journalctl -u openauto-btservice -n 50`
