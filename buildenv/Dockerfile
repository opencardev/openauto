FROM debian:buster AS prebuilts

COPY ./buildenv/pi_binaries/buster.tar.gz buster.tar.gz
COPY ./buildenv/pi_binaries/aasdk_armhf.deb aasdk_armhf.deb
RUN apt-get update &&\
    apt-get -y install cmake build-essential git zip &&\
    git clone https://github.com/SonOfGib/prebuilts.git &&\
    mkdir qt5 &&\
    cat prebuilts/qt5/Qt_5151_armv7l_OpenGLES2.tar.xz* > Qt5_OpenGLES2.tar.xz


FROM debian:buster AS openauto

COPY --from=prebuilts Qt5_OpenGLES2.tar.xz Qt5_OpenGLES2.tar.xz
COPY --from=prebuilts buster.tar.gz buster.tar.gz
COPY --from=prebuilts aasdk_armhf.deb aasdk_armhf.deb

RUN dpkg --add-architecture armhf &&\
    apt-get update &&\
    apt-get -y install cmake build-essential git &&\
    apt-get -y install gcc-arm-linux-gnueabihf cpp-arm-linux-gnueabihf g++-arm-linux-gnueabihf protobuf-compiler &&\
    apt-get -y install gcc-8-base:armhf libc6:armhf libgcc1:armhf libicu63:armhf libidn2-0:armhf libstdc++6:armhf libunistring2:armhf pulseaudio:armhf librtaudio-dev:armhf libgps-dev:armhf libblkid-dev:armhf libtag1-dev:armhf libgles2-mesa-dev:armhf libdouble-conversion-dev:armhf &&\
    tar -xf Qt5_OpenGLES2.tar.xz -C / &&\
    tar -xf buster.tar.gz -C /

# These are all the libboost requirements. It would be shorter if libboost-log-dev wasn't a disaster that's being dealt with manually.
RUN apt-get install -y libprotobuf-dev libusb-1.0.0-dev libssl-dev libboost-dev libboost-system-dev libboost-atomic1.67.0 libboost-chrono1.67.0 libboost-date-time1.67.0 libboost-filesystem1.67.0 libboost-regex1.67.0 libboost-thread1.67.0 libboost-filesystem1.67-dev libboost-thread1.67-dev libboost-date-time1.67-dev &&\
    apt-get install -y libprotobuf-dev:armhf libusb-1.0.0-dev:armhf libssl-dev:armhf libboost-dev:armhf libboost-system-dev:armhf libboost-atomic1.67.0:armhf libboost-chrono1.67.0:armhf libboost-date-time1.67.0:armhf libboost-filesystem1.67.0:armhf libboost-regex1.67.0:armhf libboost-system1.67.0:armhf libboost-thread1.67.0:armhf libboost-filesystem1.67-dev:armhf libboost-thread1.67-dev:armhf libboost-date-time1.67-dev:armhf

COPY / /src

WORKDIR /src

# Import resources
COPY ./buildenv/patch-libboost-log-deb.sh /src/resources/patch-libboost-log-deb.sh
COPY ./buildenv/entrypoint.sh /entrypoint.sh

# Patch libboost-log-dev and install
RUN chmod +x ./resources/patch-libboost-log-deb.sh
RUN ./resources/patch-libboost-log-deb.sh

# Make Executable
RUN chmod +x /entrypoint.sh

WORKDIR /
RUN dpkg-deb -xv aasdk_armhf.deb /

WORKDIR /src
ENTRYPOINT ["/bin/bash", "/entrypoint.sh"]