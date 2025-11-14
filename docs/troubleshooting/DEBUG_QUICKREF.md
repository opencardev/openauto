# Quick Reference: Remote Debugging OpenAuto

## Initial Setup (One-time)

### On Windows:
```powershell
cd C:\Users\matth\install\repos\opencardev\openauto\scripts
.\setup-remote-debug.ps1
```

### On Raspberry Pi:
```bash
chmod +x /tmp/setup-remote-debug.sh
/tmp/setup-remote-debug.sh
```

Choose option 1: "Start autoapp under gdbserver" (recommended)

## Starting a Debug Session

### 1. In VS Code:
- Press `F5`
- Select: **"Remote Debug on rpi5 (Launch)"**

### 2. Set Key Breakpoints:

**SensorService.cpp:**
```
Line ~59: stopPolling.store(true, ...)     // In stop()
Line ~226: if (!stopPolling.load(...))     // In sensorPolling()
Line ~275: void SensorService::onChannelError  // Error handler
```

**AndroidAutoEntity.cpp:**
```
Line ~64: void AndroidAutoEntity::stop()   // Main stop
Line ~320: void AndroidAutoEntity::onChannelError  // Error handler
```

### 3. Reproduce the Issue:
1. Let debugger connect and app start
2. Connect Android Auto normally
3. Click "Exit" in Android Auto
4. **Watch the breakpoints**

## When It Hangs

### Pause and Inspect:
1. Click **Pause** button (or `Ctrl+Shift+F5`)
2. Open **Call Stack** panel (View > Call Stack)
3. Look at all threads - which are active?

### Key Things to Check:

**In Variables Panel:**
- `this->stopPolling` - what's its value?
- `this->gpsEnabled_` - is it true?
- `timer_` state

**In Call Stack:**
- Is `sensorPolling()` still in the stack?
- Multiple threads showing sensor code?
- Any threads blocked on mutexes?

### Debug Console Commands:

```gdb
# List all threads
info threads

# Show all thread backtraces
thread apply all bt

# Check stopPolling value
p this->stopPolling

# Check if timer is active
p timer_

# See what thread 2 is doing
thread 2
bt

# Continue execution
continue
```

## Common Issues Found

### Issue 1: stopPolling Not Being Checked
**Symptom:** `sensorPolling()` continues after `stop()` called  
**Check:** Is the atomic load being used? Are there queued callbacks?

### Issue 2: Timer Not Cancelled
**Symptom:** New timer callbacks scheduled after `stop()`  
**Check:** Was `timer_.cancel()` called? Check error code.

### Issue 3: Channel Error Not Stopping Service
**Symptom:** Polling continues after channel error  
**Check:** Does `onChannelError()` call `stop()`?

### Issue 4: Race Condition
**Symptom:** Sometimes works, sometimes hangs  
**Check:** Thread timing between `stop()` and `sensorPolling()`

## Manual Debugging (SSH)

### Connect to Pi:
```bash
ssh pi@rpi5.home.lan
```

### Check if process is hung:
```bash
# Find process
ps aux | grep autoapp

# Check threads
ps -eLf | grep autoapp

# See what it's doing
sudo strace -p PID -f

# Get thread dump
sudo gdb -p PID -batch -ex "thread apply all bt"
```

### Stop hung process:
```bash
sudo systemctl stop openauto
# or
sudo killall -9 autoapp
```

### Start fresh debug session:
```bash
sudo gdbserver :2345 /path/to/autoapp
```

## Testing the Fix

After making code changes:

1. **Rebuild on Pi:**
   ```bash
   cd ~/src/openauto
   # Quick rebuild (if CMake already configured)
   cmake --build build-debug -j$(nproc)
   
   # Or full rebuild with build script
   ./build.sh debug
   ```

2. **Start new debug session:**
   ```bash
   sudo gdbserver :2345 ./autoapp
   ```

3. **In VS Code:** Press F5 again

4. **Test exit multiple times:**
   - Start Android Auto
   - Click Exit
   - Check logs for clean shutdown
   - Verify no timeout/hang
   - Repeat 3-5 times

## Expected Behavior (Fixed)

When you click "Exit" in Android Auto:

1. Channel errors logged (expected)
2. `onChannelError()` called
3. `stop()` called
4. `stopPolling` set to true
5. Timer cancelled
6. No more `sensorPolling()` logs
7. Clean shutdown in ~1 second
8. Service stops without SIGKILL

## Quick Tips

- **Lost connection?** Check Pi is still on: `ping rpi5.home.lan`
- **Can't set breakpoints?** Check source file paths in launch.json
- **Symbols not loading?** Verify debug build: `file ~/src/openauto/build-debug/autoapp`
- **Need to restart?** Kill gdbserver on Pi: `sudo pkill gdbserver`
- **Want clean slate?** Stop service first: `sudo systemctl stop openauto`

## File Locations

- **Windows Scripts:** `C:\Users\matth\install\repos\opencardev\openauto\scripts\`
- **VS Code Config:** `C:\Users\matth\install\repos\opencardev\openauto\.vscode\launch.json`
- **Pi Source:** `~/src/openauto/`
- **Pi Binary (debug):** `~/src/openauto/build-debug/autoapp`
- **Pi Binary (installed):** `/opt/crankshaft/autoapp`
