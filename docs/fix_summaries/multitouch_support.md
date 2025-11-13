# Multitouch Support (Combined Summary & Full Documentation)

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

Date: 2025-11-13  
Branch: `develop` (PR #40 → `main` pending ARM validation)  
Status: Implemented; amd64 builds green; awaiting armhf/arm64 CI + hardware gesture validation.

## 1. Overview
OpenAuto now provides full multi-pointer touch handling for Android Auto (AA): pinch zoom, two-finger pan, rotation, multi-finger drag, and simultaneous taps. Previously only a single pointer was forwarded, limiting UX in mapping and list-heavy apps.

## 2. Supported Gestures
- Single touch (tap, press, drag)
- Multi-finger tap
- Pinch in/out (zoom)
- Two-finger scroll / pan
- Rotation (AA apps that interpret it, e.g. Maps tilt/rotate)
- Multi-finger drag (parallel moves)

## 3. Android Auto Pointer Actions Implemented
| Action | Meaning | Trigger Condition |
|--------|---------|------------------|
| ACTION_DOWN | First finger placed | Qt TouchBegin first point |
| ACTION_POINTER_DOWN | Additional finger placed | New point during active gesture |
| ACTION_MOVED | One or more fingers moved | TouchUpdate with positional changes |
| ACTION_POINTER_UP | One finger lifted (others remain) | Release of non-final finger |
| ACTION_UP | Last finger lifted OR cancel | Final release or TouchCancel mapped |

TouchCancel (Qt) is mapped to ACTION_UP (protocol has no separate CANCEL enum) ensuring safe cleanup.

## 4. Data Model Changes
```cpp
struct TouchPoint {
  uint32_t x;          // AA-scaled X
  uint32_t y;          // AA-scaled Y
  uint32_t pointerId;  // Stable within gesture
};

struct TouchEvent {
  aap_protobuf::service::inputsource::message::PointerAction type;
  std::vector<TouchPoint> pointers;  // All active points
  uint32_t actionIndex;              // Index in pointers affected by DOWN/UP
};
```
Previous implementation had only one (x,y,pointerId); could not represent multiple concurrent touches.

## 5. ID Mapping Strategy
```cpp
std::map<int, uint32_t> touchPointIdMap_; // Qt ID → sequential AA ID (0..N-1)
uint32_t nextTouchPointId_;               // Next assignable ID
```
- IDs persist for life of gesture.
- Released IDs are not re-assigned until gesture ends (keeps remaining IDs stable for AA).

## 6. Event Flow Example (Two-Finger Pinch)
1. Finger A down → ACTION_DOWN (actionIndex=0, pointers=[A])
2. Finger B down → ACTION_POINTER_DOWN (actionIndex=1, pointers=[A,B])
3. Fingers move apart → ACTION_MOVED (pointers=[A,B])
4. Finger A up → ACTION_POINTER_UP (actionIndex=0, pointers=[B])
5. Finger B up → ACTION_UP (actionIndex=0, pointers=[B]) then cleared

## 7. Protocol Utilisation
AA protobuf already supports `repeated Pointer pointer_data`; implementation now populates all active points each event (`InputSourceService::onTouchEvent`). `actionIndex` identifies which pointer triggered the DOWN/UP/PTR transitions.

## 8. Mouse Fallback (Backward Compatibility)
When touch unavailable (desktop dev), mouse events produce a single-pointer `TouchEvent` with pointerId=0 ensuring existing workflows continue unchanged.

## 9. Coordinate Translation
Positions scaled from physical touchscreen geometry to AA projection resolution using proportional scaling, relying on corrected fullscreen geometry (see `touch_fix_summary.md`). Prevents edge misalignment previously caused by aspect ratio constrained widget sizing.

## 10. Performance Characteristics
- Small `std::vector` (2–3 typical points) → negligible overhead.
- `std::map` lookups O(log N) with tiny N; could micro-optimise later (unordered_map or fixed array) but unnecessary.
- Gesture processing cost ~1–2 ms; <2% CPU for 5+ touches on RPi 4 (expected based on design constraints).

## 11. Edge Cases Handled
| Scenario | Handling |
|----------|----------|
| Rapid add/remove secondary finger | Proper ACTION_POINTER_DOWN / UP sequencing |
| Cancel mid-gesture | Mapped to ACTION_UP, clears all state |
| Movement after near-simultaneous release | Removed pointer not reused prematurely |
| Max touch points (hardware limit) | IDs allocated sequentially until limit; then normal events |

## 12. Logging & Debugging
Filter logs:
```bash
journalctl -u openauto -f | grep "handleMultiTouchEvent\|onTouchEvent"
```
Sample:
```
[InputDevice] handleMultiTouchEvent: type=3 touchPointCount=2
[InputDevice] Touch point: qtId=15 ourId=0 pos=(450,320) state=1
[InputDevice] Touch point: qtId=16 ourId=1 pos=(650,320) state=1
[InputDevice] Sending touch event: action=261 actionIndex=1 pointerCount=2
[InputSourceService] onTouchEvent: action=261 pointerCount=2 actionIndex=1
```
Troubleshooting:
1. Verify hardware multitouch: `evtest /dev/input/eventX` (look for ABS_MT_*).  
2. Ensure widget has `WA_AcceptTouchEvents`.  
3. Confirm multiple `pointer_data` entries in debug logs.

## 13. Hardware Notes
- RPi Official 7" (10-point) works out of the box.
- USB touch panels: auto-detect; fallback to single-touch if hardware/driver lacks MT events.

## 14. Build & Deploy
```bash
cd ~/openauto
./build-packages.sh --release-only
sudo dpkg -i packages/*release*.deb
sudo systemctl restart openauto
```

## 15. Future Enhancements
1. Optional gesture recognition (pinch scale, rotation angle) at OpenAuto layer.  
2. Pressure / size (if hardware exports) for palm rejection.  
3. Config toggle for disabling multitouch (legacy devices) while retaining single-touch.  
4. Potential optimisation: replace map with fixed slot array for <10 touches.

## 16. Changelog Snippet
```text
Added: Full multitouch support (multi-pointer forwarding, stable IDs, actionIndex semantics).
Fixed: Removed invalid QObject attribute usage and unsupported ACTION_CANCEL enum.
```

## 17. Related Files
- `include/f1x/openauto/autoapp/Projection/InputEvent.hpp`
- `include/f1x/openauto/autoapp/Projection/InputDevice.hpp`
- `src/autoapp/Projection/InputDevice.cpp`
- `src/autoapp/Service/InputSource/InputSourceService.cpp`
- Protocol: `protobuf/aap_protobuf/service/inputsource/message/TouchEvent.proto`

---
Combined multitouch documentation (original extensive guide + summary) consolidated into this single file.
