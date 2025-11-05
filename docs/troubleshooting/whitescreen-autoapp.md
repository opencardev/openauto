# OpenAuto white screen in Autoapp (no Android Auto video)

This guide documents how to investigate and fix a white/blank Autoapp window when Android Auto should be rendering. It explains every step, why we do it, what to look for in logs, and lists practical fixes for each root cause.

## Summary

- Symptom: Autoapp shows a white window; audio/UI may respond but no Android Auto video.
- Typical causes:
  - Built with OpenMAX IL (OMX) on newer Raspberry Pi OS where OMX is deprecated/absent, leading to runtime failures.
  - Qt Multimedia backend present but missing H.264 decode plugins (GStreamer/libav), resulting in a player that never renders.
  - Qt platform plugin or GPU/GL stack mismatch (eglfs/xcb/wayland, EGL/GLES, vc4 KMS driver), so a surface is created but never updated.
  - Running as a service with no access to the active graphics session/DRM device.

## What we’ll do

1. Collect logs from the service or the foreground run.
2. Identify which video output backend is active (OMX vs QtVideoOutput).
3. If QtVideoOutput is used, verify codecs and services are present.
4. If OMX is used, confirm platform support; on modern OS, disable it and rebuild.
5. Validate Qt platform plugin and GPU/GL stack.
6. Apply the specific fix based on the finding.

---

## 1) Collect logs

Why: The code emits clear tags per backend and detailed error reasons.

Run (systemd managed):

```bash
journalctl -u openauto.service -b -n 500 --no-pager
```

If there’s no systemd unit or it’s run manually, search all logs:

```bash
journalctl -b -n 1000 --no-pager | grep -E "(autoapp|OpenAuto|QtVideoOutput|OMXVideoOutput|VideoMediaSinkService)"
```

Or run in the foreground (best signal):

```bash
# From your build output directory
./bin/autoapp 2>&1 | tee autoapp.log
```

What to look for:

- Backend tags:
  - Qt path: lines containing "[QtVideoOutput]" (e.g., createVideoOutput, onStartPlayback, Player started)
  - OMX path: lines containing "[OMXVideoOutput]" (e.g., omx init failed, setup tunnels failed)
- Codec/Qt errors (Qt path):
  - "[QtVideoOutput] FORMAT ERROR - This usually means a required codec is missing"
  - "[QtVideoOutput] SERVICE MISSING - GStreamer backend may not be properly installed"
- Service creation lines:
  - "[VideoMediaSinkService] getVideoResolution …" followed by open/init of videoOutput

---

## 2) Identify the active video backend

Why: Fixes differ drastically between OMX and Qt.

How to tell from logs:

- If you see any "[OMXVideoOutput] …" logs, you built with `USE_OMX` (default on Raspberry Pi builds) and the app is using the OMX pipeline.
- If you see "[QtVideoOutput] …" logs, the app is using the Qt Multimedia pipeline (`QMediaPlayer`).

Background (from source):

- In `src/autoapp/Service/ServiceFactory.cpp`, the build selects the backend at compile time:
  - When `USE_OMX` is defined, it constructs `OMXVideoOutput`.
  - Otherwise, it constructs `QtVideoOutput`.
- In `CMakeLists.txt`, `USE_OMX` is defined automatically for Raspberry Pi builds unless you set `-DNOPI=ON`.

---

## 3) If QtVideoOutput is active: verify codecs and services

Why: Qt on Linux typically uses GStreamer; missing plugins (libav, bad/ugly) will prevent H.264 playback.

Check for errors in logs:

- "[QtVideoOutput] FORMAT ERROR - This usually means a required codec is missing"
- "[QtVideoOutput] SERVICE MISSING - GStreamer backend may not be properly installed"

Fixes:

- Install GStreamer components and Qt multimedia plugins:

```bash
sudo apt-get update
sudo apt-get install -y \
  libqt5multimedia5 libqt5multimedia5-plugins \
  gstreamer1.0-tools gstreamer1.0-plugins-base gstreamer1.0-plugins-good \
  gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav
```

- Verify H.264 decoders are present:

```bash
gst-inspect-1.0 avdec_h264 || true
gst-inspect-1.0 v4l2codecs || true
```

- If running under Wayland, ensure Wayland Qt plugin is installed:

```bash
sudo apt-get install -y qtwayland5
```

- Re-run autoapp and confirm you see:
  - "[QtVideoOutput] Player started and marked ready"
  - "[QtVideoOutput] Player entered PLAYING state"

---

## 4) If OMXVideoOutput is active: address OpenMAX removal on newer OS

Why: OpenMAX IL is deprecated and removed on newer Raspberry Pi OS (Bullseye/Bookworm). The legacy `/opt/vc` stack and `ilclient` are not available or not functional with KMS/Wayland, causing white screens.

Typical errors:

- "[OMXVideoOutput] omx init failed."
- "[OMXVideoOutput] video renderer component creation failed."
- No visible Qt errors, but the window remains white.

Fix: Build without OMX to use the Qt backend.

Option A — Reconfigure without Raspberry Pi OMX flags:

```bash
mkdir -p build && cd build
cmake -DNOPI=ON -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

Option B — Explicitly remove `USE_OMX` (advanced, if you maintain a fork):

- Edit `CMakeLists.txt` to guard `USE_OMX` behind a toggle, or comment out the `add_definitions(-DUSE_OMX …)` lines.
- Rebuild; verify logs show `[QtVideoOutput]` instead of `[OMXVideoOutput]`.

After switching to QtVideoOutput, complete Step 3 to ensure codecs are installed.

---

## 5) Validate Qt platform plugin and GPU/GL stack

Why: A surface that can’t present frames (wrong plugin, missing EGL/GLES, or no access to the display) appears as a white window.

Checklist:

- Determine how Autoapp is launched:
  - As a desktop app under X11: `QT_QPA_PLATFORM=xcb` (default when a display is present)
  - Under Wayland: `QT_QPA_PLATFORM=wayland` (requires `qtwayland5`)
  - On a bare console (no X/Wayland): `QT_QPA_PLATFORM=eglfs`
- Ensure the plugin exists and loads:
  - Errors like "Could not load the Qt platform plugin …" indicate missing packages.

Packages to have installed:

```bash
sudo apt-get install -y libqt5gui5 libqt5widgets5 libqt5network5 libqt5dbus5 \
                        qtbase5-dev qtbase5-dev-tools qtwayland5
```

GPU/driver sanity checks (Raspberry Pi):

- Using KMS driver: ensure one of these overlays in `/boot/firmware/config.txt` or `/boot/config.txt`:
  - `dtoverlay=vc4-kms-v3d` (preferred)
  - `dtoverlay=vc4-fkms-v3d` (legacy fake KMS)
- Validate EGL/GLES availability:

```bash
eglinfo | head -n 50 || true
glxinfo -B || true  # if X11 present
```

- Run Autoapp as the logged-in graphical user so it can access the active seat/DRM device.

---

## 6) Quick decision guide

- Do logs show `[OMXVideoOutput]`? → Rebuild without OMX (Step 4). Then install codecs (Step 3).
- Do logs show `[QtVideoOutput]` and codec/service errors? → Install GStreamer plugins (Step 3).
- No explicit errors, still white? → Verify Qt platform plugin and GPU stack (Step 5); try setting `QT_QPA_PLATFORM` explicitly and running in foreground to capture Qt plugin loader messages.

---

## Verified fixes (apply what matches your finding)

- Rebuild without OMX on modern Raspberry Pi OS:
  - `cmake -DNOPI=ON …` → logs switch to `[QtVideoOutput]` and video renders.
- Install codecs for QtVideoOutput:
  - `gstreamer1.0-libav`, `-bad`, `-ugly`, `-base`, `-good`, and `libqt5multimedia5-plugins`.
- Install Qt Wayland plugin (if running Wayland):
  - `qtwayland5`.
- Set the correct Qt platform at launch (example):

```bash
QT_QPA_PLATFORM=wayland ./bin/autoapp
# or
QT_QPA_PLATFORM=eglfs ./bin/autoapp
```

- Ensure GPU driver is enabled (vc4 KMS) and app runs in the active graphical session.

---

## Appendix: Known log signatures from source

- Qt path emits:
  - `[QtVideoOutput] createVideoOutput()`
  - `[QtVideoOutput] Player started and marked ready`
  - `[QtVideoOutput] FORMAT ERROR - This usually means a required codec is missing`
  - `[QtVideoOutput] SERVICE MISSING - GStreamer backend may not be properly installed`

- OMX path emits:
  - `[OMXVideoOutput] omx init failed.`
  - `[OMXVideoOutput] video renderer component creation failed.`
  - `[OMXVideoOutput] setup tunnels failed.`

- Service layer:
  - `[VideoMediaSinkService] getVideoResolution …` (before videoOutput open/init)

---

## Outcome

After applying the relevant fix, you should observe in logs:

- Qt backend ready and playing, and the Autoapp window rendering Android Auto video.
- No persistent `FORMAT ERROR`/`SERVICE MISSING` messages.
- If you switched from OMX to Qt, `[OMXVideoOutput]` messages should disappear entirely.

If the issue persists, capture a fresh foreground run (`./bin/autoapp | tee log.txt`) and attach the entire log for further analysis.
