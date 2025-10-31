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
    ARCH=$(dpkg --print-architecture) && \
    echo "Attempting to install libaasdk for architecture: $ARCH" && \
    if apt-cache show libaasdk-${ARCH}-dev >/dev/null 2>&1; then \
        echo "Installing architecture-specific package: libaasdk-${ARCH}-dev"; \
        apt-get install -y --no-install-recommends libaasdk-${ARCH}-dev; \
    elif apt-cache show libaasdk-dev >/dev/null 2>&1; then \
        echo "Installing generic package: libaasdk-dev"; \
        apt-get install -y --no-install-recommends libaasdk-dev; \
    else \
        echo "ERROR: No libaasdk package found for ${ARCH}"; \
        apt-cache search libaasdk || true; \
        exit 1; \
    fi && \
    rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /src

# Copy source code
COPY . .

# Debug: List what was copied
RUN echo "Contents of /src:" && ls -la

# Create output directory for packages
RUN mkdir -p /output

# Build OpenAuto
RUN export TARGET_ARCH=$(dpkg-architecture -qDEB_HOST_ARCH) && \
        echo "Building OpenAuto for architecture: $TARGET_ARCH (native compilation)" && \
        # Determine if this is a non-Pi architecture
        CMAKE_NOPI_FLAG="" && \
        case "$TARGET_ARCH" in \
            amd64|arm64) CMAKE_NOPI_FLAG="-DNOPI=ON" ;; \
            armhf) CMAKE_NOPI_FLAG="-DNOPI=OFF" ;; \
            *) CMAKE_NOPI_FLAG="-DNOPI=ON" ;; \
        esac && \
        echo "Using CMAKE_NOPI_FLAG: $CMAKE_NOPI_FLAG" && \
        # Compute distro-specific release suffix to avoid cross-suite overwrite
        DISTRO_DEB_RELEASE=$(bash /src/scripts/distro_release.sh) && \
        CPACK_DEB_RELEASE="$DISTRO_DEB_RELEASE" && \
        echo "Using CPACK_DEBIAN_PACKAGE_RELEASE: $CPACK_DEB_RELEASE" && \
        # Configure
        env DISTRO_DEB_RELEASE="$CPACK_DEB_RELEASE" \
            cmake -S . -B build -DCMAKE_BUILD_TYPE=Release ${CMAKE_NOPI_FLAG} -DCPACK_DEBIAN_PACKAGE_RELEASE="$CPACK_DEB_RELEASE" -DCPACK_PROJECT_CONFIG_FILE=/src/cmake_modules/CPackProjectConfig.cmake && \
    # Build
    cmake --build build -j$(nproc) && \
    # Package
    cd build && \
    cpack -G DEB && \
    cd .. && \
    # Copy packages to output
    if [ -d "build" ]; then \
        find build -name "*.deb" -exec cp {} /output/ \; 2>/dev/null || true && \
        echo "Packages built:" && \
        ls -la /output/; \
    else \
        echo "No build directory found"; \
        exit 1; \
    fi && \
    echo "Build completed"

# Default command
CMD ["bash", "-c", "echo 'OpenAuto build container ready. Packages are in /output/'"]

