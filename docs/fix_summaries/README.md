<!-- Deprecated: This uppercase README retained only for case-insensitive systems. Use lowercase `readme.md` in this folder. -->
# DEPRECATED – see `readme.md`

<!--
 * Project: OpenAuto
 * This file is part of openauto project.
 * Copyright (C) 2025 OpenCarDev Team
 *
 * openauto is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * openauto is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with openauto. If not, see <http://www.gnu.org/licenses/>.
-->

This file is superseded. Refer to `readme.md` for current combined fix summaries.

## Index

| Section | Summary |
|---------|---------|
| [1. Touch Coordinate Misalignment](#1-touch-coordinate-misalignment-fix) | Full-screen geometry vs aspect ratio; corrected widget sizing for Pi 7" panel |
| [2. Shutdown Freeze Fix](#2-shutdown-freeze-fix) | Eliminated hang during Android Auto disconnect (messenger & timer races) |
| [3. Multitouch Support](#3-multitouch-support) | Added multi-pointer touch reporting, actionIndex semantics, protocol alignment |

---

## 1. Touch Coordinate Misalignment Fix

Problem: On Raspberry Pi 7" (800x480) AA video occupied ~75% width, touch hit targets offset.

Root Cause: `QVideoWidget::setFullScreen(true)` preserved source aspect ratio despite `IgnoreAspectRatio`, shrinking displayed width.

Solution (Commit: 410aa0e): Replace `setFullScreen()` with explicit geometry from `QScreen::geometry()`; fallback to fullscreen only if screen detection fails.

Key Points:

- Widget now matches physical screen exactly → coordinate scaling remains consistent.
- Touch mapping logic in `InputDevice.cpp` no longer suffers from visual vs logical width mismatch.
- No additional offset compensation required.

Impact:

- ✅ Accurate touch alignment across full width.
- ✅ Video fully occupies panel; no letterboxing.
- ✅ Works for 480p/720p/1080p negotiated AA resolutions.

See detailed original report: `docs/troubleshooting/fix_summaries/TOUCH_FIX_SUMMARY.md`.

---

## 2. Shutdown Freeze Fix

Problem: Application froze after Android Auto session ended; UI stalled, repeated errors, reconnect impossible without restart.

Root Causes:

1. AASDK `Messenger::stop()` left pending promises, triggering cascading error callbacks.
2. `SensorService` timer race allowed polling to continue post-stop.
3. Re-entrant quit logic via channel errors during teardown.
4. `QtVideoOutput` blocking calls during late write attempts.

Solutions:

- Reject pending send/receive promises with `OPERATION_ABORTED` (Commit: 3365b08 in aasdk).
- Add `stopping_` atomic guard to suppress redundant quits.
- Cancel timer immediately; triple-check `stopPolling` inside dispatched handlers.
- Early flag clear & mutex-protected writes in video output.
- Uniform handling of `OPERATION_ABORTED` across services.

Impact:

- ✅ Clean shutdown path; no runaway polling.
- ✅ Expected abort errors filtered; fatal logs only for genuine issues.
- ✅ UI remains responsive during disconnect.

See detailed original report: `docs/troubleshooting/fix_summaries/SHUTDOWN_FIX_SUMMARY.md`.

---

## 3. Multitouch Support

Problem: Single-pointer event forwarding limited user interaction (no pinch zoom, reduced gesture fidelity).

Enhancement: Refactored input pipeline to support multiple concurrent touch points with stable IDs and correct AA protocol actions.

Key Changes:

- `TouchEvent` now contains `std::vector<TouchPoint>` and `actionIndex`.
- Added `handleMultiTouchEvent(QTouchEvent*)` for `TouchBegin/Update/End/Cancel`.
- Mapped Qt IDs → sequential AA pointer IDs; recycled after full release.
- `TouchCancel` treated as `ACTION_UP` (protocol lacks cancel enum).
- Removed invalid `QObject::setAttribute` usage; fixed unsupported `ACTION_CANCEL` reference.

PointerAction Mapping:

| Gesture Transition | AA Action | Notes |
|--------------------|-----------|-------|
| First finger down | ACTION_DOWN | `actionIndex=0` |
| Additional finger down | ACTION_POINTER_DOWN | New finger index |
| One finger lifted (others remain) | ACTION_POINTER_UP | Released index |
| Last finger lifted | ACTION_UP | Clears mapping |
| Movement | ACTION_MOVED | All active points |
| Cancel | ACTION_UP | Treated as full release |

Performance:

- Small vector & map operations; negligible overhead for typical 2–3 finger gestures.

Pending Validation:

- ARM builds via PR #40 (arm64/armhf).
- Hardware test on Pi 7" (multi-finger pinch & pan).

See detailed description: `docs/fix_summaries/multitouch_support.md`.

---

## Consolidated Changelog Snippet

```text
Fixed: Touch coordinate misalignment by explicit screen geometry sizing.
Fixed: Shutdown freeze through messenger promise rejection, timer cancellation, and guarded teardown.
Added: Full multitouch support (multi-pointer touch events, stable pointer IDs, actionIndex semantics).
Fixed: Invalid QObject attribute usage and unsupported ACTION_CANCEL enum.
```

## Follow-Up Recommendations

- Add validation log (`MULTITOUCH_VALIDATION.md`) after real device gesture testing.
- Consider portrait mode & dynamic rotation support in video output.
- Optional optimisation: replace `std::map` with fixed array for pointer ID mapping.
- Introduce configuration toggle to disable multitouch for legacy panels.

## Attribution

Contributions by OpenCarDev Team; assisted implementation and documentation via automated coding agent.

---
End of combined fix summaries.
