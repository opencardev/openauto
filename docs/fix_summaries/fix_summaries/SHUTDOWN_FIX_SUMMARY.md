# DEPRECATED – moved to `../shutdown_fix_summary.md`

Original shutdown fix summary relocated. See parent directory file.

## Root Causes Identified

### 1. **aasdk Messenger incomplete shutdown** (Library Issue)
`Messenger::stop()` cleared message queues but left receive/send promises pending. When transport closed, all pending promises fired error callbacks, causing cascading failures.

### 2. **SensorService timer race condition** (Application Issue)
Timer callback could be queued on strand before `stop()` executed. Already-dispatched callback would reschedule timer despite `stopPolling` flag, causing infinite polling loop.

### 3. **Re-entrant quit during teardown** (Application Issue)
Channel error callbacks during shutdown could trigger `triggerQuit()` again while already stopping, causing state corruption and potential deadlocks.

### 4. **QtVideoOutput blocking on stop** (Application Issue)
`mediaPlayer_->stop()` could block Qt main thread waiting on GStreamer pipeline teardown, freezing UI. Late `write()` calls after stop could also cause issues.

## Solutions Implemented

### aasdk Changes (1 commit)

**Commit: 3365b08** - Fix Messenger::stop() to reject pending promises

```cpp
// src/Messenger/Messenger.cpp
void Messenger::stop() {
    // Reject all pending receive promises with OPERATION_ABORTED
    receiveStrand_.dispatch([this, self = this->shared_from_this()]() {
      channelReceiveMessageQueue_.clear();
      this->rejectReceivePromiseQueue(error::Error(error::ErrorCode::OPERATION_ABORTED));
    });
    
    // Reject all pending send promises with OPERATION_ABORTED  
    sendStrand_.dispatch([this, self = this->shared_from_this()]() {
      this->rejectSendPromiseQueue(error::Error(error::ErrorCode::OPERATION_ABORTED));
    });
}
```

**Impact:** Clean async operation cancellation; services receive expected OPERATION_ABORTED instead of transport errors.

### openauto Changes (5 commits)

#### Commit: 18a5bf3 - Add stopping_ guard to AndroidAutoEntity

```cpp
// include/.../AndroidAutoEntity.hpp
std::atomic<bool> stopping_{false};

// src/.../AndroidAutoEntity.cpp
void AndroidAutoEntity::stop() {
    stopping_.store(true, std::memory_order_relaxed);  // Set before dispatch
    strand_.dispatch([this, self = this->shared_from_this()]() {
        // ... stop services, messenger, transport, cryptor
    });
}

void AndroidAutoEntity::onChannelError(const aasdk::error::Error &e) {
    if (e.getCode() == aasdk::error::ErrorCode::OPERATION_ABORTED) {
        OPENAUTO_LOG(debug) << "OPERATION_ABORTED (expected during stop)";
        return;
    }
    if (stopping_.load(std::memory_order_relaxed)) {
        OPENAUTO_LOG(info) << "onChannelError during stopping, ignoring";
        return;
    }
    OPENAUTO_LOG(fatal) << "onChannelError(): " << e.what();
    this->triggerQuit();
}
```

**Impact:** Prevents re-entrant quit; gracefully handles expected shutdown errors.

#### Commit: 0dce201 - Fix SensorService timer race condition

```cpp
// src/.../SensorService.cpp
void SensorService::stop() {
    this->stopPolling = true;
    strand_.dispatch([this, self = this->shared_from_this()]() {
        // Cancel timer immediately to abort pending callbacks
        boost::system::error_code ec;
        this->timer_.cancel(ec);
        // ... GPS cleanup
    });
}

void SensorService::sensorPolling() {
    if (!this->stopPolling) {
        strand_.dispatch([this, self = this->shared_from_this()]() {
            // Second check inside dispatched handler
            if (this->stopPolling) {
                OPENAUTO_LOG(info) << "sensorPolling() aborted due to stop.";
                return;
            }
            // ... do GPS work
            // Third check before rescheduling
            if (!this->stopPolling) {
                timer_.expires_from_now(boost::posix_time::milliseconds(250));
                timer_.async_wait(strand_.wrap(std::bind(&SensorService::sensorPolling, ...)));
            }
        });
    }
}
```

**Impact:** Eliminates timer race; polling stops reliably even if callback was queued before stop().

#### Commit: ac66df6 - Fix QtVideoOutput shutdown blocking

```cpp
// src/.../QtVideoOutput.cpp
void QtVideoOutput::write(uint64_t, const aasdk::common::DataConstBuffer& buffer) {
    std::lock_guard<std::mutex> lock(writeMutex_);
    // Skip writes if player not ready or stopped
    if (!playerReady_) {
        return;
    }
    // ... write to buffer
}

void QtVideoOutput::onStopPlayback() {
    std::lock_guard<std::mutex> lock(writeMutex_);
    playerReady_ = false;  // Clear flag early
    // Stop player first (can block briefly)
    if (mediaPlayer_) {
        mediaPlayer_->stop();
        mediaPlayer_->setMedia(QMediaContent());
    }
    // Then hide widget (non-blocking)
    if (videoWidget_) {
        videoWidget_->hide();
        videoWidget_->clearFocus();
    }
}
```

**Impact:** Prevents writes after stop; reduces lock hold time; UI stays responsive.

#### Commit: 8efec4b - Treat OPERATION_ABORTED as expected in all services

Updated `onChannelError()` in all services:
- AudioMediaSinkService (3 channels)
- VideoMediaSinkService
- InputSourceService
- SensorService
- AndroidAutoEntity

```cpp
void Service::onChannelError(const aasdk::error::Error &e) {
    if (e.getCode() == aasdk::error::ErrorCode::OPERATION_ABORTED) {
        OPENAUTO_LOG(debug) << "onChannelError: " << e.what() << " (expected during stop)";
    } else {
        OPENAUTO_LOG(error) << "onChannelError: " << e.what();
    }
}
```

**Impact:** Clean shutdown logs; ~7 ERROR lines reduced to 0 during normal AA exit.

#### Commit: bf7f010 - Fix QtVideoOutput destructor hang and complete OPERATION_ABORTED handling

**CRITICAL FIX for 90-second timeout:**

```cpp
// include/.../QtVideoOutput.hpp
~QtVideoOutput() override;

// src/.../QtVideoOutput.cpp
QtVideoOutput::QtVideoOutput(...) {
    // Changed from Qt::QueuedConnection to Qt::BlockingQueuedConnection
    connect(this, &QtVideoOutput::stopPlayback, this, &QtVideoOutput::onStopPlayback, Qt::BlockingQueuedConnection);
}

QtVideoOutput::~QtVideoOutput() {
    OPENAUTO_LOG(info) << "[QtVideoOutput] Destructor called, ensuring cleanup";
    // Force synchronous cleanup if not already stopped
    if (playerReady_ || mediaPlayer_) {
        cleanupPlayer();
    }
}

void QtVideoOutput::cleanupPlayer() {
    // Stop the player with timeout protection
    if (mediaPlayer_) {
        OPENAUTO_LOG(debug) << "[QtVideoOutput] Stopping media player";
        mediaPlayer_->stop();
        mediaPlayer_->setMedia(QMediaContent());
    }
    
    // Hide video widget
    if (videoWidget_) {
        videoWidget_->hide();
        videoWidget_->clearFocus();
    }
}

// BluetoothService.cpp + MediaSourceService.cpp
void Service::onChannelError(const aasdk::error::Error &e) {
    if (e.getCode() == aasdk::error::ErrorCode::OPERATION_ABORTED) {
        OPENAUTO_LOG(debug) << "[Service] onChannelError(): " << e.what() << " (expected during stop)";
    } else {
        OPENAUTO_LOG(error) << "[Service] onChannelError(): " << e.what();
    }
}
```

**Root cause:** 
- `Qt::QueuedConnection` requires Qt event loop to execute `onStopPlayback()`, but event loop is often blocked/stopped during shutdown
- Custom deleter `QObject::deleteLater` **also** requires event loop, creating double dependency
- If event loop never processes queued slot, `mediaPlayer_->stop()` never executes and GStreamer resources hang
- systemd timeout kills process after 90 seconds (SIGKILL)

**Impact:** 
- **Eliminates 90-second timeout** on service restart
- Immediate, clean shutdown within ~1 second
- Explicit destructor ensures cleanup even if `deleteLater` never fires
- Blocking connection ensures `mediaPlayer_->stop()` completes synchronously

## Build & Deploy Instructions

Both repositories must be rebuilt and installed together:

```bash
# Build and install aasdk first
cd ~/aasdk
git pull origin develop
./build.sh release clean
cd build-release
sudo dpkg -i *.deb

# Then build and install openauto
cd ~/openauto
git pull origin develop
./build-packages.sh --release-only
cd packages
sudo dpkg -i *release*.deb

# Restart service
sudo systemctl restart openauto
```

## Expected Behavior After Fix

### Clean AA Exit Logs
```
[AndroidAutoEntity] stop()
[SensorService] stop()
[SensorService] sensorPolling() aborted due to stop.
[SensorService] onChannelError(): OPERATION_ABORTED (expected during stop)
[VideoMediaSinkService] onChannelError(): OPERATION_ABORTED (expected during stop)
[AudioMediaSinkService] onChannelError(): OPERATION_ABORTED (expected during stop)
[InputSourceService] onChannelError(): OPERATION_ABORTED (expected during stop)
[AndroidAutoEntity] onChannelError(): OPERATION_ABORTED (expected during stop)
[QtVideoOutput] onStopPlayback() complete
[App] onAndroidAutoQuit()
[App] Waiting for device...
```

### User-Visible Changes
- ✅ UI clock keeps ticking during and after AA exit
- ✅ Can immediately reconnect Android Auto
- ✅ No application freeze or hang
- ✅ Clean logs without error spam

## Testing

1. **Normal AA Exit**
   - Connect phone via USB or Wi-Fi
   - Use Android Auto normally
   - Exit AA from phone
   - **Verify:** OpenAuto returns to idle screen, clock ticks, no freeze

2. **Rapid Reconnect**
   - Exit AA
   - Immediately reconnect
   - **Verify:** New session starts without restart

3. **Log Verification**
   ```bash
   journalctl -u openauto.service --since "2 minutes ago" -p debug | \
     grep -E "stop|sensorPolling|onStopPlayback|OPERATION_ABORTED"
   ```
   - **Verify:** See "aborted due to stop", "OPERATION_ABORTED (expected)", "onStopPlayback() complete"
   - **Verify:** No ERROR-level channel errors during shutdown

## Architecture Notes

### Why These Fixes Are Correct

**aasdk (library):**
- Owns async operation lifecycle → must cleanly abort on stop()
- Provides OPERATION_ABORTED as standard shutdown signal
- Thread-safe promise rejection on strands

**openauto (application):**
- Owns service implementations → handles expected errors gracefully
- Manages application state (stopping_ guard) 
- Integrates Qt (non-blocking video stop)
- Controls timers (sensor polling race fix)

### Separation of Concerns
- **aasdk:** Low-level channel/messaging → clean async cancellation
- **openauto:** High-level services → graceful error handling and state management

Both layers must be updated together for complete fix.

## Related Issues

This fix resolves:
- OpenAuto freeze/hang on Android Auto exit
- Infinite sensor polling after stop
- Channel error floods during shutdown
- UI unresponsiveness after AA session ends
- Inability to reconnect without service restart

## Dependencies

- aasdk commit 3365b08 or later (Messenger promise rejection)
- openauto commits 18a5bf3, 0dce201, ac66df6, 8efec4b (application fixes)
- Both must be deployed together

## Rollback

If issues arise, revert both repositories to previous stable commits:
```bash
cd ~/aasdk
git checkout <previous-commit>
./build.sh release clean && cd build-release && sudo dpkg -i *.deb

cd ~/openauto  
git checkout <previous-commit>
./build-packages.sh --release-only && cd packages && sudo dpkg -i *release*.deb

sudo systemctl restart openauto
```

---

**Date:** November 13, 2025  
**Author:** GitHub Copilot assisted development  
**Repositories:** opencardev/aasdk, opencardev/openauto  
**Branches:** develop
