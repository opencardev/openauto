# Multi-stage Dockerfile for OpenAuto development
# Supports both amd64 and arm64 architectures

ARG BASE_IMAGE=ubuntu:22.04
FROM ${BASE_IMAGE} as base

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# Install basic system dependencies
RUN apt-get update && apt-get install -y \
    curl \
    wget \
    gnupg2 \
    lsb-release \
    software-properties-common \
    apt-transport-https \
    ca-certificates \
    locales \
    sudo \
    && rm -rf /var/lib/apt/lists/*

# Set up locale
RUN locale-gen en_US.UTF-8
ENV LANG=en_US.UTF-8
ENV LANGUAGE=en_US:en
ENV LC_ALL=en_US.UTF-8

# Install development tools and compilers
FROM base as dev-tools
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc-9 \
    g++-9 \
    cmake \
    ninja-build \
    pkg-config \
    git \
    vim \
    nano \
    htop \
    tree \
    jq \
    unzip \
    zip \
    gdb \
    valgrind \
    clang-format \
    cppcheck \
    doxygen \
    graphviz \
    ccache \
    && rm -rf /var/lib/apt/lists/*

# Set GCC 9 as default
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 60 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 60

# Install OpenAuto dependencies
FROM dev-tools as dependencies

# Install Boost libraries
RUN apt-get update && apt-get install -y \
    libboost-all-dev \
    && rm -rf /var/lib/apt/lists/*

# Install Protocol Buffers
RUN apt-get update && apt-get install -y \
    libprotobuf-dev \
    protobuf-compiler \
    && rm -rf /var/lib/apt/lists/*

# Install Qt5 development packages
RUN apt-get update && apt-get install -y \
    qtbase5-dev \
    qtmultimedia5-dev \
    qtdeclarative5-dev \
    qtquickcontrols2-5-dev \
    qml-module-qtquick2 \
    qml-module-qtquick-controls2 \
    qt5-qmake \
    qttools5-dev-tools \
    && rm -rf /var/lib/apt/lists/*

# Install audio and multimedia libraries
RUN apt-get update && apt-get install -y \
    libasound2-dev \
    libpulse-dev \
    librtaudio-dev \
    libtag1-dev \
    && rm -rf /var/lib/apt/lists/*

# Install USB and hardware interface libraries
RUN apt-get update && apt-get install -y \
    libusb-1.0-0-dev \
    libudev-dev \
    && rm -rf /var/lib/apt/lists/*

# Install JSON and HTTP libraries
RUN apt-get update && apt-get install -y \
    nlohmann-json3-dev \
    libcurl4-openssl-dev \
    && rm -rf /var/lib/apt/lists/*

# Install testing frameworks
RUN apt-get update && apt-get install -y \
    libgtest-dev \
    libgmock-dev \
    && rm -rf /var/lib/apt/lists/*

# Build and install Google Test (if not available as packages)
RUN cd /usr/src/googletest && \
    cmake . -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc) && \
    make install

# Install Python for build scripts and tools
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    python3-venv \
    && rm -rf /var/lib/apt/lists/*

RUN pip3 install --no-cache-dir \
    conan \
    cmake \
    ninja \
    pre-commit \
    black \
    isort \
    flake8

# Install Node.js and npm for web tools
RUN curl -fsSL https://deb.nodesource.com/setup_18.x | bash - && \
    apt-get install -y nodejs && \
    rm -rf /var/lib/apt/lists/*

# Build AASDK dependency
FROM dependencies as aasdk-builder

# Create workspace for AASDK
WORKDIR /tmp/aasdk

# Clone and build AASDK
RUN git clone https://github.com/opencardev/aasdk.git . && \
    mkdir protobuf/build && \
	cd protobuf/build && \
	cmake .. && \
	make && \
	make install

RUN cd /tmp/aasdk && \
	mkdir build && cd build && \
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_STANDARD=17 \
        -DCMAKE_INSTALL_PREFIX=/usr/local && \
    make -j$(nproc) && \
    make install && \
    ldconfig

# Development environment
FROM aasdk-builder as development

# Create non-root user
ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=${USER_UID}

RUN groupadd --gid ${USER_GID} ${USERNAME} && \
    useradd --uid ${USER_UID} --gid ${USER_GID} --shell /bin/bash --create-home ${USERNAME} && \
    echo "${USERNAME} ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Install additional development tools
RUN apt-get update && apt-get install -y \
    zsh \
    fish \
    tmux \
    screen \
    ripgrep \
    fd-find \
    bat \
    fzf \
    shellcheck \
    && rm -rf /var/lib/apt/lists/*

# Install Oh My Zsh for the user
USER ${USERNAME}
RUN sh -c "$(curl -fsSL https://raw.github.com/ohmyzsh/ohmyzsh/master/tools/install.sh)" "" --unattended

# Configure Git (will be overridden by user settings)
RUN git config --global init.defaultBranch main && \
    git config --global pull.rebase false && \
    git config --global core.editor "code --wait"

# Switch back to root for final setup
USER root

# Create workspace directory
WORKDIR /workspace

# Set up ccache for faster rebuilds
ENV CCACHE_DIR=/workspace/.ccache
ENV CCACHE_MAXSIZE=2G
RUN mkdir -p /workspace/.ccache && chown ${USERNAME}:${USERNAME} /workspace/.ccache

# Configure CMake to use ccache
ENV CMAKE_CXX_COMPILER_LAUNCHER=ccache
ENV CMAKE_C_COMPILER_LAUNCHER=ccache

# Install Docker CLI for container-in-container scenarios
RUN curl -fsSL https://download.docker.com/linux/ubuntu/gpg | gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg && \
    echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable" | tee /etc/apt/sources.list.d/docker.list > /dev/null && \
    apt-get update && \
    apt-get install -y docker-ce-cli && \
    rm -rf /var/lib/apt/lists/*

# Install Kubernetes tools
RUN curl -LO "https://dl.k8s.io/release/$(curl -L -s https://dl.k8s.io/release/stable.txt)/bin/linux/amd64/kubectl" && \
    install -o root -g root -m 0755 kubectl /usr/local/bin/kubectl && \
    rm kubectl

RUN curl https://raw.githubusercontent.com/helm/helm/main/scripts/get-helm-3 | bash

# Set proper permissions for workspace
RUN chown -R ${USERNAME}:${USERNAME} /workspace

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=60s --retries=3 \
    CMD gcc --version && cmake --version || exit 1

# Switch to development user
USER ${USERNAME}

# Set up environment variables
ENV EDITOR=code
ENV BROWSER=code
ENV TERM=xterm-256color

# Create common directories
RUN mkdir -p \
    /workspace/build \
    /workspace/.vscode \
    /workspace/logs \
    /workspace/config

WORKDIR /workspace

# Default command
CMD ["/bin/zsh"]
