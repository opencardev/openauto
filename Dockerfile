# * Project: OpenAuto
# * This file is part of openauto project.
# * Copyright (C) 2025 OpenCarDev Team
# *
# *  openauto is free software: you can redistribute it and/or modify
# *  it under the terms of the GNU General Public License as published by
# *  the Free Software Foundation; either version 3 of the License, or
# *  (at your option) any later version.
# *
# *  openauto is distributed in the hope that it will be useful,
# *  but WITHOUT ANY WARRANTY; without even the implied warranty of
# *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# *  GNU General Public License for more details.
# *
# *  You should have received a copy of the GNU General Public License
# *  along with openauto. If not, see <http://www.gnu.org/licenses/>.

# Multi-stage build for OpenAuto with native compilation for each architecture
# This Dockerfile builds OpenAuto natively on each target platform using QEMU emulation

# Allow selecting Debian base (bookworm or trixie). Default to trixie.
ARG DEBIAN_VERSION=trixie
FROM debian:${DEBIAN_VERSION}-slim

# Build arguments
ARG TARGET_ARCH=amd64
ARG BUILD_TYPE=release
ARG DEBIAN_FRONTEND=noninteractive

# Set locale to avoid encoding issues
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8

# Install build dependencies and tools
RUN apt-get update && apt-get install -y \
    # Core build tools
    build-essential \
    cmake \
    pkg-config \
    git \
    lsb-release \
    curl \
    gnupg \
    ca-certificates \
    # Development libraries
    libboost-system-dev \
    libboost-log-dev \
    libprotobuf-dev \
    protobuf-compiler \
    libusb-1.0-0-dev \
    libssl-dev \
    libblkid-dev \
    libgps-dev \
    libtag1-dev \
    librtaudio-dev \
    # Qt5 dependencies
    qtbase5-dev \
    qtmultimedia5-dev \
    qttools5-dev \
    qttools5-dev-tools \
    qtconnectivity5-dev \
    # Packaging tools
    file \
    dpkg-dev \
    fakeroot \
    && rm -rf /var/lib/apt/lists/*

# Add OpenCarDev APT repository for libaasdk
ARG DEBIAN_VERSION
RUN install -d -m 0755 /etc/apt/keyrings && \
        curl -fsSL https://opencardev.github.io/packages/opencardev.gpg.key -o /etc/apt/keyrings/opencardev.gpg && \
        chmod 0644 /etc/apt/keyrings/opencardev.gpg && \
        echo "deb [signed-by=/etc/apt/keyrings/opencardev.gpg] https://opencardev.github.io/packages ${DEBIAN_VERSION} main" > /etc/apt/sources.list.d/opencardev.list && \
        # Temporary safety: if requested distro is not yet published in APT repo, fall back to trixie
        if ! curl -fsSL "https://opencardev.github.io/packages/dists/${DEBIAN_VERSION}/Release" >/dev/null; then \
            echo "WARNING: OpenCarDev APT repo does not have '${DEBIAN_VERSION}'. Falling back to 'trixie' for libaasdk."; \
            echo "deb [signed-by=/etc/apt/keyrings/opencardev.gpg] https://opencardev.github.io/packages trixie main" > /etc/apt/sources.list.d/opencardev.list; \
        fi

# Install libaasdk from APT repository (attempt selected distro, fall back to trixie if not yet published)
RUN set -eux; \
    if ! apt-get update; then \
      echo "WARNING: APT update failed for ${DEBIAN_VERSION}. Falling back to 'trixie' for libaasdk source."; \
      echo "deb [signed-by=/etc/apt/keyrings/opencardev.gpg] https://opencardev.github.io/packages trixie main" > /etc/apt/sources.list.d/opencardev.list; \
      apt-get update; \
    fi; \
    if apt-cache show libaasdk >/dev/null 2>&1 && apt-cache show libaasdk-dev >/dev/null 2>&1; then \
        echo "Installing libaasdk and libaasdk-dev together"; \
        apt-get install -y --no-install-recommends libaasdk libaasdk-dev; \
    else \
        echo "ERROR: No libaasdk or libaasdk-dev package found"; \
        apt-cache search libaasdk || true; \
        exit 1; \
    fi && \
    apt-get clean && apt-get autoremove -y && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /src

# Copy source code
COPY . .

# Debug: List what was copied
RUN echo "Contents of /src:" && ls -la

# Create output directory for packages
RUN mkdir -p /output

# Make build script executable
RUN chmod +x /src/build.sh

# Build OpenAuto using unified build script
RUN /src/build.sh ${BUILD_TYPE} --package --output-dir /output

# Default command
CMD ["bash", "-c", "echo 'OpenAuto build container ready. Packages are in /output/'"]

