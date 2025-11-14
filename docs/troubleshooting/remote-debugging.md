# Remote Debugging OpenAuto on Raspberry Pi

This guide explains how to debug the OpenAuto application running on a remote Raspberry Pi from your development machine using GDB and VS Code.

## Prerequisites

- SSH access to the Raspberry Pi
- GDB installed on both development machine and Raspberry Pi
- OpenAuto built with debug symbols (`-DCMAKE_BUILD_TYPE=Debug`)
- VS Code with C/C++ extension installed

## Setup on Raspberry Pi

### 1. Install Required Packages

```bash
sudo apt-get update
sudo apt-get install -y \
    gdbserver \
    git \
    cmake \
    build-essential \
    pkg-config
```

### 2. Clone OpenAuto Repository (if not already present)

```bash
# Create source directory
mkdir -p ~/src
cd ~/src

# Clone the repository
git clone https://github.com/opencardev/openauto.git
cd openauto

# Switch to develop branch
git checkout develop
```

### 3. Install Build Dependencies

```bash
# Install aasdk dependency
sudo apt-get install -y libaasdk-dev

# Or if building aasdk from source:
cd ~/src
git clone https://github.com/opencardev/aasdk.git
cd aasdk
mkdir -p build-release
cd build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo make install

# Install other dependencies
sudo apt-get install -y \
    libboost-all-dev \
    libusb-1.0-0-dev \
    libssl-dev \
    libprotobuf-dev \
    protobuf-compiler \
    libqt5multimedia5 \
    libqt5multimedia5-plugins \
    libqt5multimediawidgets5 \
    qtmultimedia5-dev \
    libqt5bluetooth5 \
    libqt5bluetooth5-bin \
    qtconnectivity5-dev \
    pulseaudio \
    librtaudio-dev \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-plugins-ugly \
    gstreamer1.0-libav \
    gstreamer1.0-alsa \
    libgps-dev \
    gpsd \
    libblkid-dev \
    libtag1-dev
```

### 4. Build OpenAuto with Debug Symbols

```bash
cd ~/src/openauto

# Build with debug symbols using the unified build script
./build.sh debug

# The binary will be in build-debug/autoapp
# Note: Builds are done with NOPI=ON by default
# Optionally install
cd build-debug
sudo make install
```

### 3. Stop the Running Service

```bash
sudo systemctl stop openauto
```

### 4. Start Application with GDB Server

**Option A: Attach to Running Process**

```bash
# Start the application normally
sudo /path/to/autoapp &

# Find the process ID
ps aux | grep autoapp

# Attach gdbserver (replace PID with actual process ID)
sudo gdbserver --attach :2345 PID
```

**Option B: Start Application Under GDB Server**

```bash
sudo gdbserver :2345 /opt/crankshaft/autoapp
```

The application will wait for the debugger to connect before starting.

## Setup on Development Machine (Windows)

### 1. Install Required Tools

**Install GDB for Windows:**

Download from: https://www.msys2.org/

```powershell
winget install -e --id Codeblocks.Codeblocks
```

Or use WSL:

```bash
sudo apt-get install gdb-multiarch
```

### 2. Configure VS Code

Create or update `.vscode/launch.json` in your openauto workspace:

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Remote Debug OpenAuto",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/bin/autoapp",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "miDebuggerPath": "/usr/bin/gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                },
                {
                    "description": "Set Disassembly Flavor to Intel",
                    "text": "-gdb-set disassembly-flavor intel",
                    "ignoreFailures": true
                }
            ],
            "miDebuggerServerAddress": "rpi5.home.lan:2345",
            "sourceFileMap": {
                "/opt/openauto": "${workspaceFolder}",
                "/usr/src/openauto": "${workspaceFolder}"
            }
        },
        {
            "name": "Attach to Remote Process",
            "type": "cppdbg",
            "request": "attach",
            "program": "${workspaceFolder}/bin/autoapp",
            "processId": "${command:pickRemoteProcess}",
            "MIMode": "gdb",
            "miDebuggerPath": "/usr/bin/gdb",
            "miDebuggerServerAddress": "rpi5.home.lan:2345",
            "sourceFileMap": {
                "/opt/openauto": "${workspaceFolder}",
                "/usr/src/openauto": "${workspaceFolder}"
            }
        }
    ]
}
```

### 3. Set Up SSH Port Forwarding (Optional)

If direct connection doesn't work, use SSH tunneling:

```powershell
ssh -L 2345:localhost:2345 pi@rpi5.home.lan
```

Keep this terminal open while debugging.

## Debugging the Shutdown Hang

### 1. Set Breakpoints

In VS Code, set breakpoints in key locations:
 
- `src/autoapp/Service/Sensor/SensorService.cpp`:
  - Line in `stop()` method
  - Line in `sensorPolling()` method
  - Line in `onChannelError()` method
  
- `src/autoapp/Service/AndroidAutoEntity.cpp`:
  - Line in `stop()` method
  - Line in `onChannelError()` method

- `src/autoapp/App.cpp`:
  - Line in `onAndroidAutoQuit()` method

### 2. Start Debugging Session

1. Press `F5` or select "Run > Start Debugging"
2. Choose "Remote Debug OpenAuto" configuration
3. Wait for connection to establish

### 3. Reproduce the Hang

1. Use Android Auto normally
2. Click "Exit" in Android Auto
3. Observe which breakpoints are hit
4. Check the call stack and variable states

### 4. Inspect Thread State During Hang

If the application hangs:

1. Press the pause button in VS Code
2. Open the "Call Stack" panel to see all threads
3. Look for threads that are blocked or in infinite loops
4. Inspect local variables and member variables

**Key things to check:**

- `SensorService::stopPolling` value
- Timer state (`timer_.expires_at()`)
- Thread IDs and what they're waiting on
- Strand queue contents (if visible)
- io_service state

### 5. Get Thread Backtrace

In the Debug Console:

```gdb
info threads
thread apply all bt
```

This shows all threads and their call stacks.

## Common Debugging Commands

### GDB Commands in VS Code Debug Console

```gdb
# List all threads
info threads

# Switch to thread N
thread N

# Show backtrace for current thread
bt

# Show backtrace for all threads
thread apply all bt

# Print variable value
p variableName

# Print with more detail
p /x variableName

# Continue execution
continue

# Step over
next

# Step into
step

# Break on function
break SensorService::stop

# List breakpoints
info breakpoints
```

### Analyzing Deadlocks

```gdb
# Check if threads are waiting on mutexes
thread apply all bt

# Look for patterns like:
# - pthread_mutex_lock
# - boost::asio::detail::scheduler::run
# - std::condition_variable::wait
```

## SSH Commands for Manual Inspection

### Check Process State

```bash
# Find process
ps aux | grep autoapp

# Check thread count
ps -eLf | grep autoapp

# Get detailed thread info
cat /proc/PID/status

# Check open file descriptors
lsof -p PID
```

### Attach GDB Manually

```bash
# Attach to running process
sudo gdb -p PID

# Inside GDB
(gdb) info threads
(gdb) thread apply all bt
(gdb) continue
```

### Monitor System Calls

```bash
# See what system calls the process is making
sudo strace -p PID -f

# Focus on timing-related calls
sudo strace -p PID -f -e trace=futex,poll,select,epoll_wait
```

## Troubleshooting

### Cannot Connect to Remote Debugger

**Check firewall:**

```bash
# On Raspberry Pi
sudo ufw allow 2345/tcp
```

**Check gdbserver is running:**

```bash
ps aux | grep gdbserver
```

**Test connectivity:**

```powershell
Test-NetConnection -ComputerName rpi5.home.lan -Port 2345
```

### Source File Mapping Issues

If breakpoints show "Breakpoint in file that does not exist":

1. Check `sourceFileMap` in `launch.json`
2. Verify source paths on Pi: `readlink -f /proc/PID/cwd`
3. Update mapping to match actual build paths

### Symbol Loading Issues

```bash
# On Pi, check if debug symbols are present
file /path/to/autoapp
# Should show "not stripped"

# If stripped, rebuild with debug symbols
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-g3" ..
```

## Example: Debugging the SensorService Hang

### Scenario: Application hangs on exit

**Steps:**

1. Start gdbserver on Pi:
   ```bash
   sudo systemctl stop openauto
   sudo gdbserver :2345 /opt/crankshaft/autoapp
   ```

2. In VS Code:
   - Set breakpoint in `SensorService::stop()` at line where `stopPolling` is set
   - Set breakpoint in `SensorService::sensorPolling()` where it checks `stopPolling`
   - Press F5 to start debugging

3. Reproduce the issue:
   - Use Android Auto
   - Click Exit
   - Watch breakpoints

4. When hung:
   - Pause execution
   - Check "Call Stack" for all threads
   - Look for `sensorPolling` in any thread
   - Inspect `stopPolling` value in Variables panel
   - Check if timer is cancelled

5. Expected findings:
   - If `sensorPolling` is still running after `stop()`, there's a race condition
   - If multiple threads show `sensorPolling`, timer wasn't properly cancelled
   - If thread is in `timer_.async_wait`, the timer callback is pending

## Advanced: Core Dump Analysis

If the process gets killed, analyze the core dump:

```bash
# Enable core dumps
ulimit -c unlimited

# Generate core dump when killed
sudo gdb /opt/crankshaft/autoapp core

# Or manually generate
sudo gcore PID
sudo gdb /opt/crankshaft/autoapp core.PID
```

## Performance Profiling

To identify slow operations during shutdown:

```bash
# Use perf to profile
sudo perf record -p PID -g
# Trigger the hang
# Press Ctrl+C
sudo perf report
```

## References

- [GDB Remote Debugging Documentation](https://sourceware.org/gdb/onlinedocs/gdb/Remote-Debugging.html)
- [VS Code C++ Debugging](https://code.visualstudio.com/docs/cpp/cpp-debug)
- [Linux Process Debugging](https://www.kernel.org/doc/html/latest/admin-guide/README.html)
