[]: # ```plaintext
[]: #  * Project: OpenAuto
[]: #  * This file is part of openauto project.
[]: #  * Copyright (C) 2025 OpenCarDev Team
[]: #  *
[]: #  *  openauto is free software: you can redistribute it and/or modify
[]: #  *  it under the terms of the GNU General Public License as published by
[]: #  *  the Free Software Foundation; either version 3 of the License, or
[]: #  *  (at your option) any later version.
[]: #  *
[]: #  *  openauto is distributed in the hope that it will be useful,
[]: #  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
[]: #  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
[]: #  *  GNU General Public License for more details.
[]: #  *
[]: #  *  You should have received a copy of the GNU General Public License
[]: #  *  along with openauto. If not, see <http://www.gnu.org/licenses/>.
[]: # ```
# Shutdown Freeze Fix

Date: 2025-11-13  
Repos: `aasdk` & `openauto`  
Key Commits: aasdk `3365b08`; openauto `18a5bf3` `0dce201` `ac66df6` `8efec4b` `bf7f010`

## Symptoms
- UI clock stops after AA exit
- Infinite sensor polling
- Flood of channel errors (code 30)
- Cannot reconnect without restart
- Occasional 90s systemd timeout

## Root Causes
1. Messenger promises left pending (cascade errors).  
2. SensorService timer race rescheduling after stop.  
3. Re-entrant quit due to channel errors mid-teardown.  
4. Video output blocking (player stop + queued cleanup).  
5. Late writes after playback stop.

## Solutions
| Area | Fix |
|------|-----|
| Messenger | Reject pending send/receive with OPERATION_ABORTED |
| AndroidAutoEntity | `stopping_` guard; ignore expected abort errors |
| SensorService | Immediate timer cancellation; check flag before reschedule |
| QtVideoOutput | Early `playerReady_` clear; mutex-protected writes; blocking connection; destructor cleanup |
| Services | Uniform OPERATION_ABORTED handling → suppress noise |

## Result
- Clean reconnect path
- No runaway polling
- Shutdown completes ~1s
- Log noise reduced (debug-level abort lines only)

## Build & Deploy
```bash
# aasdk first
cd ~/aasdk && ./build.sh release clean && sudo dpkg -i build-release/*.deb
# then openauto
cd ~/openauto && ./build-packages.sh --release-only && sudo dpkg -i packages/*release*.deb
sudo systemctl restart openauto
```

## Expected Log Snippet
```
[AndroidAutoEntity] stop()
[SensorService] stop()
[VideoMediaSinkService] onChannelError(): OPERATION_ABORTED (expected during stop)
[QtVideoOutput] onStopPlayback() complete
```

## Rollback
Revert both repos to previous stable commits if issues arise; they must stay in sync for consistent shutdown semantics.

## Future Improvements
- Centralised cancellation abstraction for services
- Timeout wrapper for media pipeline operations
- Metrics on shutdown duration for regression detection

---
End of shutdown fix summary.
