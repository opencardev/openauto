# Multitouch Support Summary

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
Related Branch: `develop`
Related PR: #40 (develop → main)
Status: Implemented and building successfully on amd64; pending full ARM matrix validation via PR CI.

## Overview

Android Auto (AA) supports multi–pointer touch interaction (e.g. multi-finger pan and pinch gestures). Prior to this change OpenAuto only forwarded a single pointer, limiting usability (maps pinch zoom, multi-finger gestures, reliability of drag interactions). This implementation adds full multitouch support while retaining backwards compatibility for single-touch environments.

## Root Cause (Limitations Before Change)

1. `TouchEvent` structure held a single `(x, y, pointerId)` – impossible to represent concurrent points.
2. Mouse event fallback code path always sent one pointer regardless of true touch state.
3. Protocol already allowed repeated `pointer_data` in AA protobuf but the application layer never populated more than one.

## Key Changes

| Area | Change |
|------|--------|
| Data Model | Refactored `TouchEvent` to hold `std::vector<TouchPoint>` and added `actionIndex` to indicate active pointer for DOWN/UP transitions. |
| ID Mapping | Introduced stable mapping from Qt touch IDs → sequential AA pointer IDs (0..N-1) retained for the life of the gesture, recycled after all fingers lifted. |
| Event Handling | Added `handleMultiTouchEvent(QTouchEvent*)` covering `TouchBegin`, `TouchUpdate`, `TouchEnd`, and `TouchCancel` sequences. |
| Protocol Emission | Updated `InputSourceService::onTouchEvent` to loop over all `TouchPoint` entries adding multiple `pointer_data` messages. |
| Fallback | Retained existing mouse event path (`handleTouchEvent`) which now packs a single-pointer `TouchEvent` into the new container format. |
| Cancel Semantics | Qt `TouchCancel` mapped to protocol `ACTION_UP` (no native cancel enum) – clears touch state safely. |
| Compilation Fix | Removed invalid call to `QObject::setAttribute` and replaced unsupported `ACTION_CANCEL` enum usage. |

## PointerAction Mapping

| Qt State Combination | Resulting AA PointerAction | Notes |
|----------------------|----------------------------|-------|
| First finger press (TouchBegin) | ACTION_DOWN | `actionIndex = 0` |
| Additional finger press during gesture | ACTION_POINTER_DOWN | Index of newly pressed finger |
| Finger release when others remain | ACTION_POINTER_UP | Index of released finger |
| Last finger released | ACTION_UP | Clears ID map |
| Movement with one or more fingers | ACTION_MOVED | All active pointers included |
| TouchCancel (external interruption) | ACTION_UP | Treated as full release; state cleared |

## Data Structures

```cpp
struct TouchPoint {
    uint32_t x;      // Scaled to AA coordinate space
    uint32_t y;      // Scaled to AA coordinate space
    uint32_t pointerId; // Stable ID within current gesture
};

struct TouchEvent {
    aap_protobuf::service::inputsource::message::PointerAction type;
    std::vector<TouchPoint> pointers; // All active pointers
    uint32_t actionIndex;             // Index in pointers affected by DOWN/UP transitions
};
```

## Gesture Lifecycle

1. First press → assign pointerId 0; emit ACTION_DOWN.
2. Subsequent presses → assign next IDs; emit ACTION_POINTER_DOWN.
3. Movement → emit ACTION_MOVED with full list.
4. Individual release → emit ACTION_POINTER_UP and remove mapping for that ID.
5. Last release → emit ACTION_UP and reset mapping state.
6. Cancel → treat equivalently to ACTION_UP for robustness.

## Coordinate Translation

`translateTouchPoint()` scales raw Qt coordinates to match expected AA projection space using video geometry vs touchscreen geometry proportionally (avoids misalignment from aspect ratio differences previously seen with fullscreen rendering).

## Testing Performed

- Desktop (amd64) compile and run: Verified event filter receives QTouchEvent in synthetic tests.
- CI build (develop) successful for amd64 bookworm & trixie – confirms structural correctness.
- Manual inspection of generated protobuf messages (debug logging) shows multiple `pointer_data` entries when multiple fingers are active.

## Pending / To Validate

- ARM (arm64, armhf) builds in PR #40.
- Real hardware multi-finger tests on Raspberry Pi 7" display:
  - Pinch zoom in Google Maps.
  - Two-finger pan scenarios.
  - Rapid finger addition/removal edge cases.

## Edge Cases Considered

- Rapid press/release of secondary finger while primary moves.
- Cancel mid-gesture (screen orientation change, app suspension) → safe cleanup.
- Maximum simultaneous points (Qt limit vs AA expectations). Currently sequential assignment; extendable.
- Movement with one finger lifted just before others move – ensures removed ID not reused prematurely.

## Performance Considerations

Touch point vector reallocated only when count changes; typical gestures (2–3 fingers) incur minimal allocation overhead. Mapping uses `std::map` (very small N) – could be changed to `std::unordered_map` or fixed array for micro-optimisation, presently unnecessary.

## Security & Stability

All pointer indices bounds-checked before use. Cancel clears state to prevent stale IDs. No dynamic casting of QObject to QWidget for attribute calls, avoiding undefined behaviour.

## Follow-Up Recommendations

- Hardware validation log capture → add `MULTITOUCH_VALIDATION.md` with empirical results.
- Optional: Introduce protocol-level gesture hints if AA adds more actions in future.
- Consider a configuration knob to disable multitouch for legacy single-touch panels.

## Changelog Entry Suggestion

```text
Added: Full multitouch support (multi-pointer touch events) with stable pointer IDs and gesture actionIndex semantics.
Fixed: Invalid QObject attribute usage and unsupported ACTION_CANCEL enum causing CI build failures.
```

## Attribution

Implementation and fixes by OpenCarDev contributors; references to original single-touch design retained for historical context.

---
End of multitouch support summary.
