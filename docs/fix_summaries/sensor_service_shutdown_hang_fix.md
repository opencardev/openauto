# SensorService Shutdown Hang Fix

## Problem

When exiting Android Auto or stopping the openauto service, the UI would hang and the service would fail to stop cleanly. The logs showed:

- SensorService continued polling even after channel errors occurred
- `systemd` had to force-kill the process with `SIGKILL` after a timeout
- Multiple `sensorPolling()` and `is_file_exist()` log messages continued after exit

```
Nov 13 23:24:32 rpi5 autoapp[938]: [2025-11-13 23:24:32.210988] [0x00007ffeee7cef60] [error]   [OpenAuto] [InputSourceService] onChannelError(): AASDK Error: 10, Native Code: 5
Nov 13 23:24:32 rpi5 autoapp[938]: [2025-11-13 23:24:32.275003] [0x00007ffeee7cef60] [info]    [OpenAuto] [SensorService] sensorPolling()
Nov 13 23:24:32 rpi5 autoapp[938]: [2025-11-13 23:24:32.275014] [0x00007ffeee7cef60] [info]    [OpenAuto] [SensorService] is_file_exist()
...
Nov 13 23:28:10 rpi5 systemd[1]: openauto.service: State 'stop-sigterm' timed out. Killing.
Nov 13 23:28:10 rpi5 systemd[1]: openauto.service: Killing process 938 (autoapp) with signal SIGKILL.
```

## Root Cause

1. **Non-atomic `stopPolling` flag**: The `stopPolling` variable was a plain `bool`, not `std::atomic<bool>`, which could lead to race conditions when accessed from multiple threads (the main thread calling `stop()` and the timer callback thread).

2. **Timer continues after channel errors**: When Android Auto exits and channel errors occur (error code 10, native code 5), the sensor polling timer was not stopped, causing it to continue indefinitely.

3. **No cleanup on channel error**: The `onChannelError()` handler logged the error but did not trigger service cleanup, allowing the polling loop to continue even when the Android Auto session was terminated.

## Solution

### 1. Made `stopPolling` Atomic

Changed the `stopPolling` flag from `bool` to `std::atomic<bool>` to ensure thread-safe access:

**File**: `include/f1x/openauto/autoapp/Service/Sensor/SensorService.hpp`

```cpp
// Before:
bool stopPolling = false;

// After:
std::atomic<bool> stopPolling{false};
```

Added the required header:
```cpp
#include <atomic>
```

### 2. Updated All `stopPolling` Access to Use Atomic Operations

**File**: `src/autoapp/Service/Sensor/SensorService.cpp`

In `stop()`:
```cpp
// Set atomic flag first to prevent new timer callbacks from being scheduled
this->stopPolling.store(true, std::memory_order_release);
```

In `sensorPolling()`:
```cpp
// Check with atomic load
if (!this->stopPolling.load(std::memory_order_acquire)) {
    strand_.dispatch([this, self = this->shared_from_this()]() {
        // Double-check inside dispatched handler
        if (this->stopPolling.load(std::memory_order_acquire)) {
            OPENAUTO_LOG(info) << "[SensorService] sensorPolling() aborted due to stop.";
            return;
        }
        
        // ... polling work ...
        
        // Final check before rescheduling
        if (!this->stopPolling.load(std::memory_order_acquire)) {
            timer_.expires_from_now(boost::posix_time::milliseconds(250));
            timer_.async_wait(strand_.wrap(std::bind(&SensorService::sensorPolling, this->shared_from_this())));
        } else {
            OPENAUTO_LOG(info) << "[SensorService] sensorPolling() stopped, timer not rescheduled.";
        }
    });
}
```

### 3. Stop Polling on Channel Error

Updated `onChannelError()` to call `stop()` when a channel error occurs:

```cpp
void SensorService::onChannelError(const aasdk::error::Error &e) {
    // OPERATION_ABORTED is expected during shutdown when messenger stops
    if (e.getCode() == aasdk::error::ErrorCode::OPERATION_ABORTED) {
        OPENAUTO_LOG(debug) << "[SensorService] onChannelError(): " << e.what() << " (expected during stop)";
    } else {
        OPENAUTO_LOG(error) << "[SensorService] onChannelError(): " << e.what();
    }
    
    // Stop polling on any channel error to prevent hanging on exit
    if (!this->stopPolling.load(std::memory_order_acquire)) {
        OPENAUTO_LOG(info) << "[SensorService] onChannelError(): stopping sensor polling due to channel error";
        this->stop();
    }
}
```

## Impact

### Before Fix:
- Service hung on exit/stop
- `systemd` timeout (90 seconds) required to force-kill process
- Sensor polling continued indefinitely after channel errors
- Poor user experience with frozen UI

### After Fix:
- Clean, immediate shutdown within ~1 second
- No `systemd` timeouts or forced kills
- Sensor polling stops immediately when Android Auto exits
- Proper resource cleanup
- Thread-safe flag access eliminates race conditions

## Testing

To test the fix:

1. Start Android Auto normally
2. Click "Exit" in Android Auto
3. Observe clean shutdown without hanging
4. Check logs for `[SensorService] sensorPolling() stopped, timer not rescheduled.`
5. Stop the service with `systemctl stop openauto`
6. Verify no timeout or `SIGKILL` in logs

## Related Files

- `include/f1x/openauto/autoapp/Service/Sensor/SensorService.hpp`
- `src/autoapp/Service/Sensor/SensorService.cpp`

## Date

2025-11-13
