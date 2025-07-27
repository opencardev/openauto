# Podman + WSL Troubleshooting Guide

## Common Issues and Solutions

### Issue: WSL Wayland Socket Mount Error
```
Error: getting absolute path of \\wsl.localhost\Debian\mnt\wslg\runtime-dir\wayland-0: unsupported UNC path
```

**Cause**: VS Code Dev Containers extension automatically tries to mount WSL GUI sockets for X11/Wayland forwarding, but Podman on Windows doesn't support UNC paths.

**Solution**: The devcontainer configurations have been simplified to avoid this issue entirely by:
- Removing custom workspace mounts
- Using default volume mounting behavior  
- Avoiding GUI forwarding requirements

### Issue: Container Platform Not Supported
```
Error: platform not supported
```

**Solution for ARM builds**:
1. Enable Podman emulation:
   ```powershell
   podman machine set --qemu-user-static-path=auto
   ```

2. Or use Docker Desktop with BuildKit enabled:
   ```powershell
   set DOCKER_BUILDKIT=1
   ```

### Issue: Container Won't Start
```
Error: command failed: podman run...
```

**Troubleshooting steps**:
1. Ensure Podman machine is running:
   ```powershell
   podman machine start
   ```

2. Check available space:
   ```powershell
   podman system df
   ```

3. Clean up if needed:
   ```powershell
   podman system prune -f
   ```

### Issue: Build Fails with Permission Errors
**Solution**: Try running VS Code as Administrator or check Podman permissions:
```powershell
podman machine ssh
# Inside the machine:
sudo chown -R root:root /var/lib/containers
```

## Verification Commands

Test your Podman setup:
```powershell
# Check Podman version
podman --version

# Test simple container
podman run --rm hello-world

# Test platform emulation (for ARM builds)
podman run --rm --platform linux/arm64 alpine echo "ARM64 works"
```

## Alternative: Switch to Docker
If Podman continues to cause issues, switch to Docker Desktop:

1. Install Docker Desktop
2. In VS Code settings, remove Podman configuration:
   - Open Settings (Ctrl+,)
   - Search for "dev containers docker"
   - Remove any custom docker/compose paths
3. Restart VS Code

The devcontainer configurations work with both Docker and Podman.
