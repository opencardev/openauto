
# OpenAuto

### Support project
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=R4HXE5ESDR4U4)

For support of other platforms please contact me at f1xstudiopl@gmail.com

### Community
[![Join the chat at https://gitter.im/publiclab/publiclab](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/openauto_androidauto/Lobby)

### Description
OpenAuto is an AndroidAuto(tm) headunit emulator based on aasdk library and Qt libraries. Main goal is to run this application on the RaspberryPI 3 board computer smoothly.

[See demo video](https://www.youtube.com/watch?v=k9tKRqIkQs8)

### Supported functionalities
 - 480p, 720p and 1080p with 30 or 60 FPS
 - RaspberryPI 3 hardware acceleration support to decode video stream (up to 1080p@60!)
 - Audio playback from all audio channels (Media, System and Speech)
 - Audio input for voice commands
 - Touchscreen and buttons input
 - Bluetooth
 - Automatic launch after device hotplug
 - Automatic detection of connected Android devices
 - Wireless (WiFi) mode via head unit server (must be enabled in hidden developer settings)
 - User-friendly settings

### Supported platforms

 - Linux
 - RaspberryPI 3
 - Windows

### License
GNU GPLv3

Copyrights (c) 2018 f1x.studio (Michal Szwaj)

*AndroidAuto is registered trademark of Google Inc.*

### Used software
 - [aasdk](https://github.com/f1xpl/aasdk)
 - [Boost libraries](http://www.boost.org/)
 - [Qt libraries](https://www.qt.io/)
 - [CMake](https://cmake.org/)
 - [RtAudio](https://www.music.mcgill.ca/~gary/rtaudio/playback.html)
 - Broadcom ilclient from RaspberryPI 3 firmware
 - OpenMAX IL API

### Building
#### Amd64
Install the packages specified in the [prebuilts](https://github.com/opencardev/prebuilts) repository. Qt5 is required, versions packaged in modern Ubuntu and Debian
seem to work fine.

You will also likely need to install the udev rules from `prebuilts`

You need to point some CMAKE variables at your `aasdk` files.
```text
-DAASDK_INCLUDE_DIRS=<path_to_aasdk_repo>/include
-DAASDK_LIBRARIES=<path_to_aasdk_repo>/lib/libaasdk.so
 DAASDK_PROTO_INCLUDE_DIRS=<path_to_aasdk_build>
-DAASDK_PROTO_LIBRARIES=<path_to_aasdk_repo>/lib/libaasdk_proto.so
```

#### Raspberry Pi
Just run the scripts in the `prebuilts` repository for `aasdk` and `openauto`. It is possible to cross compile if your raspberry pi is too slow to compile the code itself.
However, its easiest to just develop on a more capable `amd64` device.

### Remarks
**This software is not certified by Google Inc. It is created for R&D purposes and may not work as expected by the original authors. Do not use while driving. You use this software at your own risk.**
