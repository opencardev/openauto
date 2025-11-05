# OpenAuto Troubleshooting Template

Use this template to diagnose and fix issues. Replace placeholders as you go.

## 1) Issue summary

- Symptom: <describe exact symptom>
- Expected: <what should happen>
- Environment:
  - Hardware: <e.g., Raspberry Pi 4>
  - OS: <e.g., Raspberry Pi OS Bookworm 64-bit>
  - Desktop/session: <X11 | Wayland | console>
  - Launch mode: <systemd service | manual>
  - OpenAuto version/build: <commit or package>

## 2) Collect logs

Why: Logs show backend selection and concrete error reasons.

Run (prefer foreground if possible):

```bash
./bin/autoapp 2>&1 | tee autoapp.log
```

If running as a service:

```bash
journalctl -u openauto.service -b -n 500 --no-pager
```

Search for key tags:

```bash
grep -E "(QtVideoOutput|OMXVideoOutput|VideoMediaSinkService|Could not load the Qt platform plugin)" autoapp.log || true
```

Record findings:

- Active backend: [ QtVideoOutput | OMXVideoOutput | Unknown ]
- Errors observed: <paste lines>

## 3) Backend decision branch

- If logs contain `[OMXVideoOutput]` → go to Section 4 (OMX path)
- If logs contain `[QtVideoOutput]` → go to Section 5 (Qt path)
- If neither appears → go to Section 6 (Qt platform/GL stack)

## 4) OMX path (Raspberry Pi legacy)

Why: OpenMAX IL is deprecated on newer OS images; failures cause blank/white video.

Checklist:

- Look for: `omx init failed` / `video renderer component creation failed` / `setup tunnels failed`.
- Action: Rebuild without OMX to force Qt backend.

Commands:

```bash
mkdir -p build && cd build
cmake -DNOPI=ON -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

Verify after rebuild:

- Logs show `[QtVideoOutput]` instead of `[OMXVideoOutput]`.

## 5) Qt path (GStreamer/codec)

Why: Missing codecs or services prevent H.264 playback.

Install required packages:

```bash
sudo apt-get update
sudo apt-get install -y \
  libqt5multimedia5 libqt5multimedia5-plugins \
  gstreamer1.0-tools gstreamer1.0-plugins-base gstreamer1.0-plugins-good \
  gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav
```

Validate decoders:

```bash
gst-inspect-1.0 avdec_h264 || true
gst-inspect-1.0 v4l2codecs || true
```

Expected healthy logs:

- `[QtVideoOutput] Player started and marked ready`
- `[QtVideoOutput] Player entered PLAYING state`

If errors persist, paste:

- `[QtVideoOutput] FORMAT ERROR …` or `[QtVideoOutput] SERVICE MISSING …`

## 6) Qt platform plugin and GPU/GL stack

Why: Wrong/missing platform plugin (xcb/wayland/eglfs) or lack of EGL/GLES prevents rendering.

- Determine session type: `echo $XDG_SESSION_TYPE`
- Set platform explicitly and retry foreground run:

```bash
QT_QPA_PLATFORM=wayland ./bin/autoapp   # if Wayland
QT_QPA_PLATFORM=xcb ./bin/autoapp       # if X11
QT_QPA_PLATFORM=eglfs ./bin/autoapp     # if no compositor
```

- Ensure packages:

```bash
sudo apt-get install -y libqt5gui5 libqt5widgets5 qtbase5-dev qtbase5-dev-tools qtwayland5
```

- GPU driver checks (Raspberry Pi): enable `dtoverlay=vc4-kms-v3d` (or `vc4-fkms-v3d`).
- Validate EGL/GLES:

```bash
eglinfo | head -n 50 || true
glxinfo -B || true
```

## 7) Fix applied

- Root cause: <selected from above>
- Actions taken: <commands/changes>
- Result: <now working? include key log lines>

## 8) Follow-ups / notes

- <Optional performance tuning, e.g., set 720p/30fps to start>
- <Record package versions installed>
- <Attach full log if still broken>
