cmake_minimum_required(VERSION 3.5.1)

project(openauto CXX)

message(STATUS "OpenAuto")

option(NOPI "Build for Non Raspberry Pi" OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Set Compile Versions
string(TIMESTAMP OPENAUTO_BUILD_DATE "%Y%m%d")  # Dynamic Binary Release Build Date
set(OPENAUTO_BUILD_MAJOR_RELEASE 5)             # Binary Release Build Major Number
set(OPENAUTO_BUILD_MINOR_RELEASE 0)             # Binary Release Build Minor Number
set(OPENAUTO_BUILD_INCREMENTAL 0)               # Binary Build Version - Increment for Each Build

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

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules/")

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
    find_package(Protobuf REQUIRED)
endif ()

find_package(aap_protobuf REQUIRED)
find_package(aasdk REQUIRED)
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

set(PROGRAM_VERSION_STRING "${OPENAUTO_BUILD_MAJOR_RELEASE}.${OPENAUTO_BUILD_MINOR_RELEASE}.${OPENAUTO_BUILD_INCREMENTAL}+${OPENAUTO_BUILD_DATE}")

message(STATUS "Project Version: ${PROGRAM_VERSION_STRING}")

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
        PROPERTIES VERSION ${PROGRAM_VERSION_STRING} SOVERSION ${OPENAUTO_BUILD_INCREMENTAL})

# Install rules
#install(TARGETS autoapp btservice
#        EXPORT aasdkTargets
#        RUNTIME DESTINATION bin
#)

# Modern Architecture Components
option(ENABLE_MODERN_API "Enable modern REST API and event bus" ON)

if(ENABLE_MODERN_API)
    message(STATUS "Modern API enabled")
    
    # Find required packages for modern components
    find_package(PkgConfig REQUIRED)
    
    # Check for nlohmann/json
    find_path(NLOHMANN_JSON_INCLUDE_DIRS nlohmann/json.hpp
        PATHS /usr/include /usr/local/include /opt/homebrew/include
        PATH_SUFFIXES nlohmann)
    
    if(NOT NLOHMANN_JSON_INCLUDE_DIRS)
        message(FATAL_ERROR "nlohmann/json not found. Please install nlohmann-json-dev package.")
    endif()
    
    # Check for httplib
    find_path(HTTPLIB_INCLUDE_DIRS httplib.h
        PATHS /usr/include /usr/local/include /opt/homebrew/include)
    
    if(NOT HTTPLIB_INCLUDE_DIRS)
        message(WARNING "httplib.h not found. You may need to install cpp-httplib manually.")
    endif()
    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DENABLE_MODERN_API")
    
    # Modern sources
    set(modern_sources_directory ${sources_directory}/modern)
    set(modern_include_directory ${include_directory}/modern)
    
    file(GLOB_RECURSE modern_source_files 
        ${modern_sources_directory}/*.cpp 
        ${modern_include_directory}/*.hpp)
    
    # Add modern components to autoapp
    target_sources(autoapp PRIVATE ${modern_source_files})
    target_include_directories(autoapp PRIVATE ${NLOHMANN_JSON_INCLUDE_DIRS} ${HTTPLIB_INCLUDE_DIRS})
    
    # Add threading support for REST API
    find_package(Threads REQUIRED)
    target_link_libraries(autoapp PUBLIC Threads::Threads)
    
    message(STATUS "Modern API components added to build")
endif()