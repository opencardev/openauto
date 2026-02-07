#!/bin/bash

# Project: Crankshaft
# This file is part of Crankshaft project.
# Copyright (C) 2025 OpenCarDev Team
#
#  Crankshaft is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
#
#  Crankshaft is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Crankshaft. If not, see <http://www.gnu.org/licenses/>.

set -eu

# Add OpenCarDev repository GPG key and repository configuration
echo "Adding OpenCarDev repository..."

# install pre-reqs
echo "Installing prerequisites..."
apt update
apt install curl gpg lsb-release -y

# Create directory for GPG keys if it doesn't exist
mkdir -p "/usr/share/keyrings"
chmod 755 "/usr/share/keyrings"

# Download and add the GPG key
curl -fsSL https://apt.opencardev.org/opencardev.gpg.key | \
    gpg --dearmor > "/usr/share/keyrings/opencardev-archive-keyring.gpg"

# Set proper permissions on the keyring
chmod 644 "/usr/share/keyrings/opencardev-archive-keyring.gpg"

echo "OpenCarDev GPG key added successfully"
echo "Configuring OpenCarDev repository sources..."

# Get architecture and release codename
ARCH=$(dpkg --print-architecture)
CODENAME=$(lsb_release -cs)

# Create the repository configuration
cat > /etc/apt/sources.list.d/opencardev.list << EOF
deb [arch=${ARCH} signed-by=/usr/share/keyrings/opencardev-archive-keyring.gpg] https://apt.opencardev.org ${CODENAME} stable
EOF

echo "OpenCarDev repository configured: $(cat /etc/apt/sources.list.d/opencardev.list)"
# Update package lists
echo "Updating package lists..."
apt-get update

echo "OpenCarDev repository setup completed successfully"
