cmake_minimum_required(VERSION 3.14)

project(openauto CXX)

message(STATUS "OpenAuto")

# Versioning: align with AASDK (YYYY.MM.DD[+git.<sha>[.dirty]][.debug])
# Date-based semantic versioning
string(TIMESTAMP OPENAUTO_BUILD_YEAR "%Y" UTC)
string(TIMESTAMP OPENAUTO_BUILD_MONTH "%m" UTC)
string(TIMESTAMP OPENAUTO_BUILD_DAY "%d" UTC)

option(NOPI "Build for Non Raspberry Pi" OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Set version components
set(OPENAUTO_BUILD_MAJOR_RELEASE ${OPENAUTO_BUILD_YEAR})
set(OPENAUTO_BUILD_MINOR_RELEASE ${OPENAUTO_BUILD_MONTH})
set(OPENAUTO_BUILD_PATCH_RELEASE ${OPENAUTO_BUILD_DAY})

# Configure CMAKE
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_INIT} -Wall -pedantic -fPIC")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-g -O3")

# Default to Release mode unless overridden with -DCMAKE_BUILD_TYPE=Debug on cmake command
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Forcing Release build type")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,/usr/local/qt5/lib")

    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Paths
set(resources_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets)
set(sources_directory ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(include_directory ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules/")

# Get git commit information using gitversion.cmake module
set(local_dir ${CMAKE_CURRENT_SOURCE_DIR})
include(gitversion)
# Include DEB package filename generation module
include(DebPackageFilename)

# Configure Boost
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

add_definitions(-DBOOST_ALL_DYN_LINK)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Disabling Boost DEBUG logs")
    add_definitions(-DNDEBUG)
endif()

if (WIN32)
    set(WINSOCK2_LIBRARIES "ws2_32")
endif (WIN32)

if (NOPI)
    message(STATUS "Configuring for Non-Raspberry Pi")
else()
    message(STATUS "Configuring for RaspberryPi")
    add_definitions(-DUSE_OMX -DOMX_SKIP64BIT -DRASPBERRYPI3)
    set(BCM_HOST_LIBRARIES "/opt/vc/lib/libbcm_host.so")
    set(BCM_HOST_INCLUDE_DIRS "/opt/vc/include")
    set(ILCLIENT_INCLUDE_DIRS "/opt/vc/src/hello_pi/libs/ilclient")
    set(ILCLIENT_LIBRARIES "/opt/vc/src/hello_pi/libs/ilclient/libilclient.a;/opt/vc/lib/libvcos.so;/opt/vc/lib/libvcilcs.a;/opt/vc/lib/libvchiq_arm.so")
endif ()

# Building on a Mac requires Abseil
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")  # macOS
    message(STATUS "MacOS System Detected")
    add_definitions(-DMAC_OS)
    find_package(protobuf REQUIRED CONFIG)
    find_package(absl REQUIRED)
    set(CMAKE_PREFIX_PATH "/usr/local/opt/qt@5" ${CMAKE_PREFIX_PATH})
    list(APPEND CMAKE_PREFIX_PATH /usr/local/opt/util-linux)
    list(APPEND CMAKE_PREFIX_PATH /usr/local/opt/taglib)
    include(FindProtobuf)
else ()
    # Linux specific command
    find_path(Protobuf_INCLUDE_DIR google/protobuf/message.h PATHS /usr/include /usr/local/include)
    find_library(Protobuf_LIBRARY protobuf PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu /usr/lib/arm-linux-gnueabihf /usr/local/lib)
    find_program(PROTOBUF_PROTOC_EXECUTABLE protoc)
    message(STATUS "Found Protobuf include: ${Protobuf_INCLUDE_DIR}")
    message(STATUS "Found Protobuf library: ${Protobuf_LIBRARY}")
    message(STATUS "Found protoc: ${PROTOBUF_PROTOC_EXECUTABLE}")
    if(NOT Protobuf_INCLUDE_DIR OR NOT Protobuf_LIBRARY OR NOT PROTOBUF_PROTOC_EXECUTABLE)
        message(FATAL_ERROR "Protobuf not found. Please install libprotobuf-dev and protobuf-compiler")
    endif()
    set(Protobuf_FOUND TRUE)
    set(PROTOBUF_LIBRARIES ${Protobuf_LIBRARY})
    set(PROTOBUF_INCLUDE_DIR ${Protobuf_INCLUDE_DIR})
endif ()

find_package(aap_protobuf)
find_package(aasdk)

if(NOT AAP_PROTOBUF_FOUND OR NOT AASDK_FOUND)
message(FATAL_ERROR "aap_protobuf or aasdk not found. Please build and install AASDK first.")
endif()

find_package(Boost REQUIRED COMPONENTS system log_setup log OPTIONAL_COMPONENTS unit_test_framework)
find_package(libusb-1.0 REQUIRED)
find_package(Qt5 COMPONENTS DBus Multimedia MultimediaWidgets Bluetooth Network)
find_package(OpenSSL REQUIRED)
find_package(rtaudio REQUIRED)
find_package(taglib REQUIRED)
find_package(blkid REQUIRED)
find_package(gps REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
        ${Qt5Multimedia_INCLUDE_DIRS}
        ${Qt5MultimediaWidgets_INCLUDE_DIRS}
        ${Qt5Widgets_INCLUDE_DIRS}
        ${Qt5Bluetooth_INCLUDE_DIRS}
        ${Qt5Network_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
        ${OPENSSL_INCLUDE_DIR}
        ${RTAUDIO_INCLUDE_DIRS}
        ${TAGLIB_INCLUDE_DIRS}
        ${BLKID_INCLUDE_DIRS}
        ${BCM_HOST_INCLUDE_DIRS}
        ${ILCLIENT_INCLUDE_DIRS}
        ${include_directory}
        ${PROTOBUF_INCLUDE_DIR}
        ${AAP_PROTOBUF_INCLUDE_DIR}
        ${AASDK_INCLUDE_DIR})

link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

set(common_include_directory ${include_directory}/f1x/openauto/Common)

set(autoapp_sources_directory ${sources_directory}/autoapp)
set(autoapp_include_directory ${include_directory}/f1x/openauto/autoapp)

file(GLOB_RECURSE autoapp_source_files ${autoapp_sources_directory}/*.ui ${autoapp_sources_directory}/*.cpp ${autoapp_include_directory}/*.hpp ${autoapp_include_directory}/*.h ${common_include_directory}/*.hpp ${resources_directory}/*.qrc)

add_executable(autoapp ${autoapp_source_files})

target_include_directories(autoapp PUBLIC ${AAP_PROTOBUF_INCLUDE_DIR} ${AASDK_INCLUDE_DIR})

target_link_libraries(autoapp PUBLIC
        libusb
        ${Boost_LIBRARIES}
        ${Qt5DBus_LIBRARIES}
        ${Qt5Multimedia_LIBRARIES}
        ${Qt5MultimediaWidgets_LIBRARIES}
        ${Qt5Bluetooth_LIBRARIES}
        ${Qt5Network_LIBRARIES}
        ${BCM_HOST_LIBRARIES}
        ${ILCLIENT_LIBRARIES}
        ${WINSOCK2_LIBRARIES}
        ${RTAUDIO_LIBRARIES}
        ${TAGLIB_LIBRARIES}
        ${BLKID_LIBRARIES}
        ${GPS_LIBRARIES}
        ${PROTOBUF_LIBRARIES}
        ${AAP_PROTOBUF_LIB_DIR}
        ${AASDK_LIB_DIR})

set(PROGRAM_VERSION_STRING "${OPENAUTO_BUILD_MAJOR_RELEASE}.${OPENAUTO_BUILD_MINOR_RELEASE}.${OPENAUTO_BUILD_PATCH_RELEASE}")

# Add git commit info to version if available
if(GIT_COMMIT_ID AND NOT GIT_COMMIT_ID STREQUAL "unknown")
    set(PROGRAM_VERSION_STRING "${PROGRAM_VERSION_STRING}+git.${GIT_COMMIT_ID}")
    if(GIT_DIRTY GREATER 0)
        set(PROGRAM_VERSION_STRING "${PROGRAM_VERSION_STRING}.dirty")
    endif()
endif()

# Add build type suffix for debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(PROGRAM_VERSION_STRING "${PROGRAM_VERSION_STRING}.debug")
endif()

message(STATUS "Project Version: ${PROGRAM_VERSION_STRING}")
message(STATUS "Git Branch: ${GIT_BRANCH}")
message(STATUS "Git Commit: ${GIT_COMMIT_ID}")
message(STATUS "Git Describe: ${GIT_DESCRIBE}")
message(STATUS "Git Timestamp: ${GIT_COMMIT_TIMESTAMP}")
if(GIT_DIRTY GREATER 0)
    message(STATUS "Repository Status: DIRTY (has uncommitted changes)")
else()
    message(STATUS "Repository Status: CLEAN")
endif()

set(btservice_sources_directory ${sources_directory}/btservice)

set(btservice_include_directory ${include_directory}/f1x/openauto/btservice)
file(GLOB_RECURSE btservice_source_files ${btservice_sources_directory}/*.cpp ${btservice_include_directory}/*.hpp ${autoapp_sources_directory}/Configuration/*.cpp ${autoapp_includes_directory}/Configuration/*.hpp ${common_include_directory}/*.hpp)

add_executable(btservice ${btservice_source_files}
        src/btservice/BluetoothHandler.cpp)

target_link_libraries(btservice PUBLIC
        ${Boost_LIBRARIES}
        ${Qt5Bluetooth_LIBRARIES}
        ${Qt5Network_LIBRARIES}
        ${Qt5MultimediaWidgets_LIBRARIES}
        ${PROTOBUF_LIBRARIES}
        ${AAP_PROTOBUF_LIB_DIR})

set_target_properties(autoapp
    PROPERTIES VERSION ${PROGRAM_VERSION_STRING} SOVERSION ${OPENAUTO_BUILD_MAJOR_RELEASE})

# Install rules
install(TARGETS autoapp btservice
    RUNTIME DESTINATION bin
)

# Install udev rules
install(FILES packaging/udev/99-openauto.rules
    DESTINATION /lib/udev/rules.d
)

# Install systemd service files
install(FILES 
    packaging/systemd/openauto.service
    packaging/systemd/openauto-btservice.service
    DESTINATION /lib/systemd/system
)

# Install crankshaft scripts to /opt/crankshaft
install(FILES
    packaging/opt/crankshaft/crankshaft_default_env.sh
    packaging/opt/crankshaft/crankshaft_system_env.sh
    packaging/opt/crankshaft/usb_action.sh
    DESTINATION /opt/crankshaft
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

# Install crankshaft launcher to /usr/local/bin
install(FILES
    helpers/crankshaft
    helpers/autoapp_helper
    DESTINATION /usr/local/bin
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

# Packaging configuration (DEB via CPack)
# Auto-detect architecture for proper Debian packaging
execute_process(
    COMMAND dpkg --print-architecture
    OUTPUT_VARIABLE DETECTED_ARCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
if(DETECTED_ARCH)
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${DETECTED_ARCH}")
    message(STATUS "Detected architecture for packaging: ${DETECTED_ARCH}")
else()
    # Fallback to system processor
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7l")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "armhf")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i386|i686")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
    else()
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "unknown")
    endif()
    message(STATUS "Fallback architecture detection: ${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
endif()

set(CPACK_GENERATOR "DEB")

# Add debug suffix to package name for debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CPACK_PACKAGE_NAME "openauto-dbg")
else()
    set(CPACK_PACKAGE_NAME "openauto")
endif()

set(CPACK_PACKAGE_VENDOR "OpenCarDev")
set(CPACK_PACKAGE_CONTACT "OpenCarDev Team <opencardevuk@gmail.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenAuto head unit application using libaasdk")
set(CPACK_PACKAGE_DESCRIPTION "OpenAuto is an AndroidAuto head unit emulator designed for Raspberry Pi and other embedded platforms.")
set(CPACK_PACKAGE_VERSION "${PROGRAM_VERSION_STRING}")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/opencardev/openauto")

# Debian package configuration
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "OpenCarDev Team <opencardevuk@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/opencardev/openauto")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

# Explicit dependencies for GStreamer video codec support and Qt Multimedia backend
# These are required for H.264/H.265 video decoding from Android Auto via QtVideoOutput
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5multimedia5, libqt5multimedia5-plugins, gstreamer1.0-plugins-base, gstreamer1.0-plugins-good, gstreamer1.0-libav, gstreamer1.0-plugins-bad, gstreamer1.0-plugins-ugly")

# Debian package control scripts
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    "${CMAKE_CURRENT_SOURCE_DIR}/packaging/debian/postinst"
    "${CMAKE_CURRENT_SOURCE_DIR}/packaging/debian/prerm"
    "${CMAKE_CURRENT_SOURCE_DIR}/packaging/debian/postrm"
    "${CMAKE_CURRENT_SOURCE_DIR}/packaging/debian/preinst"
)

# Configure DEB package filename using the DebPackageFilename module
configure_deb_filename(
    PACKAGE_NAME "${CPACK_PACKAGE_NAME}"
    VERSION "${PROGRAM_VERSION_STRING}"
    ARCHITECTURE "${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}"
)

# Optional: add basic metadata
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

# Source package configuration
set(CPACK_SOURCE_GENERATOR "TGZ;TBZ2")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${PROGRAM_VERSION_STRING}-src")
set(CPACK_SOURCE_IGNORE_FILES
    "/build/"
    "/build-debug/"
    "/[.]git/"
    "/[.]vscode/"
    "/[.]idea/"
    "[.]gitignore"
    "[.]DS_Store"
    "/lib/"
    "/bin/"
    "[.]so$"
    "[.]a$"
)

include(CPack)