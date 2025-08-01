# Container Deployment Guide

This document provides comprehensive guidance for deploying OpenAuto using containerization technologies including Docker, Podman, and Kubernetes.

## Table of Contents
- [Overview](#overview)
- [Docker Deployment](#docker-deployment)
- [Multi-Architecture Builds](#multi-architecture-builds)
- [Kubernetes Deployment](#kubernetes-deployment)
- [Container Security](#container-security)
- [Performance Optimization](#performance-optimization)
- [Monitoring and Logging](#monitoring-and-logging)
- [Troubleshooting](#troubleshooting)

## Overview

Containerized deployment offers several benefits for OpenAuto:
- **Consistency**: Same environment across development, testing, and production
- **Isolation**: Dependencies contained within the container
- **Scalability**: Easy horizontal scaling with orchestration
- **Portability**: Run on any container-compatible platform
- **Version Control**: Tagged images for version management

### Deployment Scenarios
1. **Development Environment**: Local development with hot reload
2. **Testing Environment**: Automated testing in CI/CD
3. **Production Single-Node**: Docker Compose deployment
4. **Production Multi-Node**: Kubernetes orchestration
5. **Edge Deployment**: Lightweight containers for embedded systems

## Docker Deployment

### Base Images

#### Development Image
```dockerfile
# Dockerfile.dev
FROM ubuntu:22.04 as development

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    git \
    libboost-all-dev \
    libprotobuf-dev \
    protobuf-compiler \
    nlohmann-json3-dev \
    qtbase5-dev \
    qtmultimedia5-dev \
    libgtest-dev \
    libusb-1.0-0-dev \
    libtag1-dev \
    librtaudio-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /workspace

# Copy source code
COPY . .

# Configure build
RUN cmake -B build -S . \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTS=ON \
    -DENABLE_COVERAGE=ON

# Build
RUN cmake --build build --parallel $(nproc)

# Development server
EXPOSE 8080
CMD ["./build/bin/autoapp", "--config", "/workspace/config/development.json"]
```

#### Production Image (Multi-stage)
```dockerfile
# Dockerfile.prod
FROM ubuntu:22.04 as builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    git \
    libboost-all-dev \
    libprotobuf-dev \
    protobuf-compiler \
    nlohmann-json3-dev \
    qtbase5-dev \
    qtmultimedia5-dev \
    libusb-1.0-0-dev \
    libtag1-dev \
    librtaudio-dev \
    && rm -rf /var/lib/apt/lists/*

# Build AASDK
WORKDIR /tmp/aasdk
RUN git clone https://github.com/opencardev/aasdk.git . && \
    mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DTARGET_ARCH=amd64 && \
    make -j$(nproc) && \
    make install

# Copy source code
WORKDIR /src
COPY . .

# Build OpenAuto
RUN cmake -B build -S . \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=OFF \
    -DCMAKE_INSTALL_PREFIX=/opt/openauto

RUN cmake --build build --parallel $(nproc)
RUN cmake --install build

# Production image
FROM ubuntu:22.04 as production

# Install runtime dependencies only
RUN apt-get update && apt-get install -y \
    libboost-system1.74.0 \
    libboost-filesystem1.74.0 \
    libboost-thread1.74.0 \
    libboost-log1.74.0 \
    libboost-program-options1.74.0 \
    libprotobuf23 \
    libqt5core5a \
    libqt5widgets5 \
    libqt5multimedia5 \
    libusb-1.0-0 \
    libtag1v5 \
    librtaudio6 \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user
RUN useradd -r -s /bin/false openauto

# Copy built application
COPY --from=builder /opt/openauto /opt/openauto
COPY --from=builder /usr/local/lib/libaasdk* /usr/local/lib/

# Update library cache
RUN ldconfig

# Create directories
RUN mkdir -p /var/log/openauto /etc/openauto && \
    chown openauto:openauto /var/log/openauto

# Copy configuration
COPY config/production.json /etc/openauto/config.json

# Switch to non-root user
USER openauto

# Health check
HEALTHCHECK --interval=30s --timeout=5s --start-period=60s --retries=3 \
    CMD curl -f http://localhost:8080/api/health || exit 1

# Expose ports
EXPOSE 8080 5555

# Start application
CMD ["/opt/openauto/bin/autoapp", "--config", "/etc/openauto/config.json"]
```

#### ARM64/Raspberry Pi Image
```dockerfile
# Dockerfile.arm64
FROM arm64v8/ubuntu:22.04 as builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    git \
    libboost-all-dev \
    libprotobuf-dev \
    protobuf-compiler \
    nlohmann-json3-dev \
    qtbase5-dev \
    qtmultimedia5-dev \
    libusb-1.0-0-dev \
    libtag1-dev \
    librtaudio-dev \
    # Raspberry Pi specific
    wiringpi \
    libraspberrypi-dev \
    && rm -rf /var/lib/apt/lists/*

# ARM-specific build flags
ENV CMAKE_CXX_FLAGS="-mcpu=cortex-a72 -mfpu=neon-fp-armv8 -O3"

# ... rest similar to production build ...

# Production ARM64 image
FROM arm64v8/ubuntu:22.04 as production-arm64

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libboost-system1.74.0 \
    libboost-filesystem1.74.0 \
    libboost-thread1.74.0 \
    libboost-log1.74.0 \
    libboost-program-options1.74.0 \
    libprotobuf23 \
    libqt5core5a \
    libqt5widgets5 \
    libqt5multimedia5 \
    libusb-1.0-0 \
    libtag1v5 \
    librtaudio6 \
    # ARM/Pi specific runtime
    wiringpi \
    && rm -rf /var/lib/apt/lists/*

# ... rest similar to production image ...
```

### Docker Compose Configuration

#### Development Stack
```yaml
# docker-compose.dev.yml
version: '3.8'

services:
  openauto-dev:
    build:
      context: .
      dockerfile: Dockerfile.dev
    volumes:
      - .:/workspace
      - cmake-cache:/workspace/build
      - /dev/bus/usb:/dev/bus/usb
    ports:
      - "8080:8080"
      - "5555:5555"
    environment:
      - CMAKE_BUILD_TYPE=Debug
      - OPENAUTO_LOG_LEVEL=DEBUG
    devices:
      - /dev/dri:/dev/dri
    privileged: true
    command: >
      bash -c "
        cmake --build build --parallel $(nproc) &&
        ./build/bin/autoapp --config config/development.json
      "

  redis:
    image: redis:7-alpine
    ports:
      - "6379:6379"
    volumes:
      - redis-data:/data

volumes:
  cmake-cache:
  redis-data:
```

#### Production Stack
```yaml
# docker-compose.prod.yml
version: '3.8'

services:
  openauto:
    build:
      context: .
      dockerfile: Dockerfile.prod
    restart: unless-stopped
    ports:
      - "8080:8080"
      - "5555:5555"
    volumes:
      - ./config/production.json:/etc/openauto/config.json:ro
      - ./logs:/var/log/openauto
      - /dev/bus/usb:/dev/bus/usb
    environment:
      - OPENAUTO_LOG_LEVEL=INFO
      - OPENAUTO_CONFIG_PATH=/etc/openauto/config.json
    devices:
      - /dev/dri:/dev/dri
    privileged: true
    healthcheck:
      test: ["CMD", "curl", "-f", "http://localhost:8080/api/health"]
      interval: 30s
      timeout: 5s
      retries: 3
      start_period: 60s

  nginx:
    image: nginx:alpine
    restart: unless-stopped
    ports:
      - "80:80"
      - "443:443"
    volumes:
      - ./nginx/nginx.conf:/etc/nginx/nginx.conf:ro
      - ./nginx/ssl:/etc/nginx/ssl:ro
    depends_on:
      - openauto

  redis:
    image: redis:7-alpine
    restart: unless-stopped
    volumes:
      - redis-data:/data
    command: redis-server --appendonly yes

volumes:
  redis-data:
```

## Multi-Architecture Builds

### BuildKit Multi-Platform
```bash
# Create builder
docker buildx create --name multiarch --driver docker-container --use

# Build for multiple architectures
docker buildx build --platform linux/amd64,linux/arm64,linux/arm/v7 \
  -t openauto/openauto:latest \
  -f Dockerfile.prod \
  --push .
```

### Platform-Specific Build Scripts
```bash
#!/bin/bash
# build-multiarch.sh

set -e

REGISTRY="openauto"
IMAGE_NAME="openauto"
VERSION=${1:-latest}

# Platforms to build for
PLATFORMS="linux/amd64,linux/arm64,linux/arm/v7"

echo "Building multi-architecture image: ${REGISTRY}/${IMAGE_NAME}:${VERSION}"

# Create and use buildx builder
docker buildx create --name multiarch --driver docker-container --use --bootstrap

# Build and push
docker buildx build \
  --platform ${PLATFORMS} \
  --tag ${REGISTRY}/${IMAGE_NAME}:${VERSION} \
  --tag ${REGISTRY}/${IMAGE_NAME}:latest \
  --file Dockerfile.prod \
  --push \
  .

# Clean up builder
docker buildx rm multiarch

echo "Multi-architecture build complete!"
```

## Kubernetes Deployment

### Namespace and ConfigMap
```yaml
# kubernetes/namespace.yaml
apiVersion: v1
kind: Namespace
metadata:
  name: openauto
  labels:
    name: openauto

---
apiVersion: v1
kind: ConfigMap
metadata:
  name: openauto-config
  namespace: openauto
data:
  config.json: |
    {
      "logger": {
        "level": "INFO",
        "file": "/var/log/openauto/app.log",
        "console": true
      },
      "api": {
        "host": "0.0.0.0",
        "port": 8080,
        "cors": true
      },
      "android_auto": {
        "video_fps": 60,
        "video_resolution": "1920x1080",
        "audio_sample_rate": 48000
      }
    }
```

### Deployment and Service
```yaml
# kubernetes/deployment.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: openauto
  namespace: openauto
  labels:
    app: openauto
spec:
  replicas: 1
  selector:
    matchLabels:
      app: openauto
  template:
    metadata:
      labels:
        app: openauto
    spec:
      containers:
      - name: openauto
        image: openauto/openauto:latest
        ports:
        - containerPort: 8080
          name: http
        - containerPort: 5555
          name: android-auto
        env:
        - name: OPENAUTO_CONFIG_PATH
          value: "/etc/openauto/config.json"
        - name: OPENAUTO_LOG_LEVEL
          value: "INFO"
        volumeMounts:
        - name: config
          mountPath: /etc/openauto
          readOnly: true
        - name: logs
          mountPath: /var/log/openauto
        - name: usb
          mountPath: /dev/bus/usb
        livenessProbe:
          httpGet:
            path: /api/health
            port: 8080
          initialDelaySeconds: 60
          periodSeconds: 30
        readinessProbe:
          httpGet:
            path: /api/ready
            port: 8080
          initialDelaySeconds: 10
          periodSeconds: 5
        resources:
          requests:
            memory: "512Mi"
            cpu: "500m"
          limits:
            memory: "2Gi"
            cpu: "2000m"
        securityContext:
          runAsNonRoot: true
          runAsUser: 1000
          allowPrivilegeEscalation: false
          capabilities:
            drop:
            - ALL
            add:
            - NET_BIND_SERVICE
      volumes:
      - name: config
        configMap:
          name: openauto-config
      - name: logs
        emptyDir: {}
      - name: usb
        hostPath:
          path: /dev/bus/usb
          type: Directory
      nodeSelector:
        openauto/device-type: "automotive"
      tolerations:
      - key: "automotive"
        operator: "Equal"
        value: "true"
        effect: "NoSchedule"

---
apiVersion: v1
kind: Service
metadata:
  name: openauto-service
  namespace: openauto
  labels:
    app: openauto
spec:
  selector:
    app: openauto
  ports:
  - name: http
    port: 80
    targetPort: 8080
    protocol: TCP
  - name: android-auto
    port: 5555
    targetPort: 5555
    protocol: TCP
  type: LoadBalancer
```

### Ingress Configuration
```yaml
# kubernetes/ingress.yaml
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: openauto-ingress
  namespace: openauto
  annotations:
    nginx.ingress.kubernetes.io/rewrite-target: /
    nginx.ingress.kubernetes.io/ssl-redirect: "true"
    cert-manager.io/cluster-issuer: "letsencrypt-prod"
spec:
  tls:
  - hosts:
    - openauto.example.com
    secretName: openauto-tls
  rules:
  - host: openauto.example.com
    http:
      paths:
      - path: /
        pathType: Prefix
        backend:
          service:
            name: openauto-service
            port:
              number: 80
```

### Persistent Storage
```yaml
# kubernetes/storage.yaml
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: openauto-logs
  namespace: openauto
spec:
  accessModes:
    - ReadWriteOnce
  resources:
    requests:
      storage: 10Gi
  storageClassName: fast-ssd

---
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: openauto-data
  namespace: openauto
spec:
  accessModes:
    - ReadWriteOnce
  resources:
    requests:
      storage: 50Gi
  storageClassName: standard
```

## Container Security

### Security Best Practices
```dockerfile
# Security-hardened Dockerfile
FROM ubuntu:22.04 as base

# Create non-root user early
RUN groupadd -r openauto && useradd -r -g openauto openauto

# Install security updates
RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y --no-install-recommends \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# ... build steps ...

# Final security configuration
USER openauto

# Remove unnecessary tools
RUN apt-get remove -y \
    build-essential \
    git \
    && apt-get autoremove -y \
    && apt-get clean

# Set secure permissions
RUN chmod -R 750 /opt/openauto

# Health check with timeout
HEALTHCHECK --interval=30s --timeout=5s --start-period=60s --retries=3 \
    CMD timeout 3s curl -f http://localhost:8080/api/health || exit 1
```

### Security Scanning
```yaml
# .github/workflows/security.yml
name: Container Security Scan

on:
  push:
    branches: [ main ]
  pull_request:
    branches: [ main ]

jobs:
  scan:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v3
    
    - name: Build image
      run: docker build -t openauto:test .
    
    - name: Run Trivy vulnerability scanner
      uses: aquasecurity/trivy-action@master
      with:
        image-ref: 'openauto:test'
        format: 'sarif'
        output: 'trivy-results.sarif'
    
    - name: Upload Trivy scan results
      uses: github/codeql-action/upload-sarif@v2
      with:
        sarif_file: 'trivy-results.sarif'
```

## Performance Optimization

### Multi-stage Build Optimization
```dockerfile
# Optimized multi-stage build
FROM ubuntu:22.04 as dependencies

# Install and cache dependencies
RUN apt-get update && apt-get install -y \
    libboost-system1.74.0 \
    libboost-filesystem1.74.0 \
    && rm -rf /var/lib/apt/lists/*

FROM dependencies as builder

# Install build tools only in builder
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    && rm -rf /var/lib/apt/lists/*

# Build application
COPY . /src
WORKDIR /src
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --parallel $(nproc)

FROM dependencies as runtime

# Copy only runtime artifacts
COPY --from=builder /src/build/bin /opt/openauto/bin
COPY --from=builder /src/config /opt/openauto/config

# Optimize image size
RUN find /opt/openauto -type f -executable -exec strip {} \;

USER openauto
CMD ["/opt/openauto/bin/autoapp"]
```

### Container Resource Limits
```yaml
# kubernetes/resource-limits.yaml
resources:
  requests:
    memory: "256Mi"
    cpu: "250m"
    ephemeral-storage: "1Gi"
  limits:
    memory: "1Gi"
    cpu: "1000m"
    ephemeral-storage: "2Gi"
```

## Monitoring and Logging

### Logging Configuration
```yaml
# docker-compose.logging.yml
version: '3.8'

services:
  openauto:
    # ... service configuration ...
    logging:
      driver: "json-file"
      options:
        max-size: "100m"
        max-file: "3"
        labels: "service,environment"
    labels:
      - "service=openauto"
      - "environment=production"

  loki:
    image: grafana/loki:latest
    ports:
      - "3100:3100"
    volumes:
      - ./loki:/etc/loki
    command: -config.file=/etc/loki/loki.yml

  promtail:
    image: grafana/promtail:latest
    volumes:
      - /var/log:/var/log:ro
      - ./promtail:/etc/promtail
    command: -config.file=/etc/promtail/promtail.yml

  grafana:
    image: grafana/grafana:latest
    ports:
      - "3000:3000"
    environment:
      - GF_SECURITY_ADMIN_PASSWORD=admin
    volumes:
      - grafana-data:/var/lib/grafana

volumes:
  grafana-data:
```

### Prometheus Monitoring
```yaml
# kubernetes/monitoring.yaml
apiVersion: v1
kind: ServiceMonitor
metadata:
  name: openauto-metrics
  namespace: openauto
spec:
  selector:
    matchLabels:
      app: openauto
  endpoints:
  - port: metrics
    interval: 30s
    path: /metrics
```

## Troubleshooting

### Common Issues

#### Container Won't Start
```bash
# Check container logs
docker logs openauto-container

# Inspect container
docker inspect openauto-container

# Check health
docker exec openauto-container curl http://localhost:8080/api/health
```

#### USB Device Access
```yaml
# docker-compose.yml USB access
services:
  openauto:
    devices:
      - /dev/bus/usb:/dev/bus/usb
    privileged: true
    # Or specific USB device
    # devices:
    #   - /dev/bus/usb/001/002:/dev/bus/usb/001/002
```

#### Performance Issues
```bash
# Monitor container resources
docker stats openauto-container

# Check container processes
docker exec openauto-container ps aux

# Analyze container filesystem
docker exec openauto-container df -h
```

#### Network Connectivity
```bash
# Test network connectivity
docker exec openauto-container curl -v http://localhost:8080/api/health

# Check port bindings
docker port openauto-container

# Network debugging
docker exec openauto-container netstat -tlnp
```

### Debugging Tools
```dockerfile
# Debug image with additional tools
FROM openauto/openauto:latest as debug

USER root

RUN apt-get update && apt-get install -y \
    curl \
    netcat \
    strace \
    gdb \
    htop \
    && rm -rf /var/lib/apt/lists/*

USER openauto
```

## Best Practices

### Image Optimization
1. Use multi-stage builds
2. Minimize layer count
3. Remove build dependencies from final image
4. Use .dockerignore file
5. Strip debug symbols from binaries

### Security Hardening
1. Run as non-root user
2. Use minimal base images
3. Regular security scanning
4. Keep dependencies updated
5. Implement proper secret management

### Deployment Strategy
1. Blue-green deployments
2. Rolling updates
3. Health checks and readiness probes
4. Resource limits and requests
5. Horizontal pod autoscaling

## Related Documentation

- [Deployment Guide](deployment-guide.md) - General deployment strategies
- [Hardware Setup](hardware-setup.md) - Physical hardware configuration
- [Monitoring Guide](monitoring-guide.md) - System monitoring
- [Performance Guide](performance-guide.md) - Performance optimization
