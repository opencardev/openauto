#!/bin/bash
# OpenAuto Post-Installation Setup Script

set -e

echo "🚀 OpenAuto Post-Installation Setup"

# Create openauto user and group if they don't exist
if ! getent group openauto >/dev/null; then
    echo "Creating openauto group..."
    groupadd --system openauto
fi

if ! getent passwd openauto >/dev/null; then
    echo "Creating openauto user..."
    useradd --system --gid openauto --home-dir /opt/openauto --shell /bin/false openauto
fi

# Add openauto user to required groups
echo "Adding openauto user to required groups..."
usermod -a -G audio,video,bluetooth,dialout,input,plugdev openauto

# Create and set permissions for directories
echo "Setting up directories and permissions..."

# Application directory
chown -R openauto:openauto /opt/openauto
chmod 755 /opt/openauto

# Data directory
mkdir -p /var/lib/openauto
chown -R openauto:openauto /var/lib/openauto
chmod 755 /var/lib/openauto

# Log directory
mkdir -p /var/log/openauto
chown -R openauto:openauto /var/log/openauto
chmod 750 /var/log/openauto

# Configuration directory permissions
chown -R root:openauto /etc/openauto
chmod 755 /etc/openauto
chmod 640 /etc/openauto/*.conf

# Reload udev rules
echo "Reloading udev rules..."
udevadm control --reload-rules
udevadm trigger

# Reload systemd
echo "Reloading systemd daemon..."
systemctl daemon-reload

# Enable services
echo "Enabling OpenAuto services..."
systemctl enable openauto.service
systemctl enable openauto-btservice.service

echo "✅ OpenAuto setup complete!"
echo ""
echo "📝 Next steps:"
echo "   1. Configure audio/video devices in /etc/openauto/openauto.conf"
echo "   2. Start services: sudo systemctl start openauto"
echo "   3. Check status: sudo systemctl status openauto"
echo "   4. View logs: sudo journalctl -u openauto -f"
echo ""
echo "🌐 REST API will be available at: http://localhost:8080"
