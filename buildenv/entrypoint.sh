#!/bin/bash

# Clear out the /build directory
mkdir build;
cd build;
cmake -DCMAKE_BUILD_TYPE=Release -DRPI3_BUILD=true -DCROSS_COMPILE_ARMHF=true -DAASDK_INCLUDE_DIRS=/usr/include/aasdk -DAASDK_LIBRARIES=/usr/lib/libaasdk.so -DAASDK_PROTO_INCLUDE_DIRS=/usr/include/aasdk_proto -DAASDK_PROTO_LIBRARIES=/usr/lib/libaasdk_proto.so ../
make -j4
cd ../bin

# Move it to release
rm -f /release/autoapp
rm -f /release/btservice
mv autoapp /release
mv btservice /release