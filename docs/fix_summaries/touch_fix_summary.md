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
# Touch Coordinate Misalignment Fix

Date: 2025-11-13  
Commit: 410aa0e  
Branch: `develop`

## Problem
AA video filled ~75% width on RPi 7" (800x480); touch targets horizontally offset.

## Root Cause
`QVideoWidget::setFullScreen(true)` enforced aspect ratio despite `IgnoreAspectRatio`, shrinking usable width. Touch scaling assumed full physical width → mismatch.

## Solution
Replaced fullscreen with explicit widget geometry:
```cpp
QScreen* screen = QGuiApplication::primaryScreen();
if(screen) {
    videoWidget_->setGeometry(screen->geometry());
} else {
    videoWidget_->setFullScreen(true); // fallback
}
```
Maintains `IgnoreAspectRatio` so AA content stretches to fill 800x480.

## Impact
| Before | After |
|--------|-------|
| Video ~600x480 | Video 800x480 |
| Touch at physical 75% hits wrong logical target | 1:1 alignment |
| Visible offset | No offset |

## Testing
1. Tap buttons across width → direct hits.  
2. Check logs for geometry line.  
3. Validate multi-resolution (480p/720p/1080p) negotiation: alignment intact.

## Why Correct
Widget now truly matches physical screen, eliminating need for compensation logic; simpler and robust across resolutions.

## Future Ideas
- Optional aspect ratio preservation toggle.  
- Portrait/rotation support.  
- Configurable margins/padding.

---
End of touch fix summary.