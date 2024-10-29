### RPI Cross Compile
The docker image in this folder is intended to be used to cross compile `openauto` without having to configure your
host pc with multiarch or installing a toolchain.

#### Setup
The `openauto` build for RPI3 requires some files from the PI, as well as aasdk libraries compiled for amrhf.

 - RPI files should be compressed to `buildenv/pi_binaries/buster.tar.gz`. The files required in the archive should
match the path/files in the `if(RPI3_BUILD)` section of `CMakeLists.txt`
 - Copy the `.deb` file from `aasdk` docker cross compile build and place it in `buildenv/pi_binaries/aasdk_armhf.dev`
#### Build
```bash
cd buildenv
sudo docker compose up --build
```
Binary files will then be available in `buildenv/release`.