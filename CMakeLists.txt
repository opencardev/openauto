cmake_minimum_required(VERSION 3.5.1)

project(openauto CXX)

message(STATUS "OpenAuto")

option(NOPI "Build for Non Raspberry Pi" OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Set Compile Versions (Date-based versioning: YYYY.MM.DD+gitcommit)
string(TIMESTAMP OPENAUTO_BUILD_YEAR "%Y")      # Year as major version
string(TIMESTAMP OPENAUTO_BUILD_MONTH "%m")     # Month as minor version (01-12)
string(TIMESTAMP OPENAUTO_BUILD_DAY "%d")       # Day as patch version (01-31)
string(TIMESTAMP OPENAUTO_BUILD_DATE "%Y%m%d")  # Full date for compatibility

# Get git commit information
find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE OPENAUTO_GIT_COMMIT_SHORT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE OPENAUTO_GIT_COMMIT_FULL
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --dirty --always
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE OPENAUTO_GIT_DESCRIBE
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE OPENAUTO_GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
else()
    set(OPENAUTO_GIT_COMMIT_SHORT "unknown")
    set(OPENAUTO_GIT_COMMIT_FULL "unknown")
    set(OPENAUTO_GIT_DESCRIBE "unknown")
    set(OPENAUTO_GIT_BRANCH "unknown")
    message(WARNING "Git not found. Version will not include commit information.")
endif()

# Set version numbers using date-based scheme
set(OPENAUTO_BUILD_MAJOR_RELEASE ${OPENAUTO_BUILD_YEAR})   # Year (e.g., 2025)
set(OPENAUTO_BUILD_MINOR_RELEASE ${OPENAUTO_BUILD_MONTH})  # Month (01-12)
set(OPENAUTO_BUILD_INCREMENTAL ${OPENAUTO_BUILD_DAY})      # Day (01-31)

# Configure CMAKE
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_INIT} -Wall -pedantic -fPIC")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-g -O3")

# Add version and git information as compile definitions
add_definitions(-DOPENAUTO_VERSION_MAJOR=${OPENAUTO_BUILD_MAJOR_RELEASE})
add_definitions(-DOPENAUTO_VERSION_MINOR=${OPENAUTO_BUILD_MINOR_RELEASE})
add_definitions(-DOPENAUTO_VERSION_PATCH=${OPENAUTO_BUILD_INCREMENTAL})
add_definitions(-DOPENAUTO_VERSION_STRING="${PROGRAM_VERSION_STRING}")
add_definitions(-DOPENAUTO_GIT_COMMIT="${OPENAUTO_GIT_COMMIT_SHORT}")
add_definitions(-DOPENAUTO_GIT_COMMIT_FULL="${OPENAUTO_GIT_COMMIT_FULL}")
add_definitions(-DOPENAUTO_GIT_BRANCH="${OPENAUTO_GIT_BRANCH}")
add_definitions(-DOPENAUTO_GIT_DESCRIBE="${OPENAUTO_GIT_DESCRIBE}")
add_definitions(-DOPENAUTO_BUILD_DATE="${OPENAUTO_BUILD_DATE}")

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

# PNG Profile Check and Fix Function
function(check_and_fix_png_profiles ASSETS_DIR)
    message(STATUS "🔧 Checking PNG files for problematic iCCP profiles...")
    
    # Find all PNG files
    file(GLOB PNG_FILES "${ASSETS_DIR}/*.png")
    
    # Check if required tools are available
    find_program(PNGCHECK_EXECUTABLE pngcheck)
    find_program(PNGCRUSH_EXECUTABLE pngcrush)
    find_program(CONVERT_EXECUTABLE convert)
    
    if(NOT PNGCHECK_EXECUTABLE)
        message(WARNING "⚠️  pngcheck not found. Install with: sudo apt-get install pngcheck")
        return()
    endif()
    
    set(PROBLEMATIC_FILES "")
    set(FIXED_COUNT 0)
    
    foreach(PNG_FILE ${PNG_FILES})
        get_filename_component(FILENAME ${PNG_FILE} NAME)
        
        # Check if file has iCCP profile
        execute_process(
            COMMAND ${PNGCHECK_EXECUTABLE} -v ${PNG_FILE}
            OUTPUT_VARIABLE PNGCHECK_OUTPUT
            ERROR_VARIABLE PNGCHECK_ERROR
            RESULT_VARIABLE PNGCHECK_RESULT
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        
        if(PNGCHECK_OUTPUT MATCHES "iCCP")
            list(APPEND PROBLEMATIC_FILES ${PNG_FILE})
            message(WARNING "🚨 PROBLEMATIC PNG DETECTED:")
            message(WARNING "   📄 File: ${FILENAME}")
            message(WARNING "   📁 Path: ${PNG_FILE}")
            message(WARNING "   ⚠️  This file contains an incorrect sRGB profile that will cause:")
            message(WARNING "      'libpng warning: iCCP: known incorrect sRGB profile'")
            message(WARNING "   🔧 Attempting automatic fix...")
            
            # Try to fix with pngcrush
            if(PNGCRUSH_EXECUTABLE)
                execute_process(
                    COMMAND ${PNGCRUSH_EXECUTABLE} -rem iCCP ${PNG_FILE} ${PNG_FILE}.tmp
                    OUTPUT_QUIET
                    ERROR_QUIET
                    RESULT_VARIABLE PNGCRUSH_RESULT
                )
                
                if(PNGCRUSH_RESULT EQUAL 0)
                    file(RENAME ${PNG_FILE}.tmp ${PNG_FILE})
                    message(STATUS "   ✅ Fixed ${FILENAME} using pngcrush")
                    math(EXPR FIXED_COUNT "${FIXED_COUNT} + 1")
                elseif(CONVERT_EXECUTABLE)
                    # Fallback to ImageMagick
                    execute_process(
                        COMMAND ${CONVERT_EXECUTABLE} ${PNG_FILE} -strip ${PNG_FILE}.tmp
                        OUTPUT_QUIET
                        ERROR_QUIET
                        RESULT_VARIABLE CONVERT_RESULT
                    )
                    
                    if(CONVERT_RESULT EQUAL 0)
                        file(RENAME ${PNG_FILE}.tmp ${PNG_FILE})
                        message(STATUS "   ✅ Fixed ${FILENAME} using ImageMagick")
                        math(EXPR FIXED_COUNT "${FIXED_COUNT} + 1")
                    else()
                        message(WARNING "   ❌ Failed to fix ${FILENAME}")
                        file(REMOVE_IF_EXISTS ${PNG_FILE}.tmp)
                    endif()
                endif()
            else()
                message(WARNING "   ❌ pngcrush not available. Install with: sudo apt-get install pngcrush")
            endif()
        endif()
    endforeach()
    
    list(LENGTH PROBLEMATIC_FILES PROBLEMATIC_COUNT)
    if(PROBLEMATIC_COUNT GREATER 0)
        message(STATUS "🎯 PNG Profile Check Summary:")
        message(STATUS "   📊 Problematic files found: ${PROBLEMATIC_COUNT}")
        message(STATUS "   🔧 Files automatically fixed: ${FIXED_COUNT}")
        
        if(FIXED_COUNT LESS PROBLEMATIC_COUNT)
            message(WARNING "⚠️  Some PNG files still have problematic profiles!")
            message(WARNING "   Install fixing tools: sudo apt-get install pngcrush imagemagick")
        else()
            message(STATUS "✅ All PNG profile issues resolved!")
        endif()
    else()
        message(STATUS "✅ All PNG files are clean - no profile issues found")
    endif()
endfunction()

# Run PNG profile check on assets
check_and_fix_png_profiles(${resources_directory})

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

# Remove any legacy EventBus files to prevent conflicts with modern implementation
list(FILTER autoapp_source_files EXCLUDE REGEX ".*/EventBus/.*")
list(FILTER autoapp_source_files EXCLUDE REGEX ".*EventBus.*")

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

set(PROGRAM_VERSION_STRING "${OPENAUTO_BUILD_MAJOR_RELEASE}.${OPENAUTO_BUILD_MINOR_RELEASE}.${OPENAUTO_BUILD_INCREMENTAL}+${OPENAUTO_GIT_COMMIT_SHORT}")

message(STATUS "Project Version: ${PROGRAM_VERSION_STRING}")
message(STATUS "Git Information:")
message(STATUS "  Branch: ${OPENAUTO_GIT_BRANCH}")
message(STATUS "  Commit: ${OPENAUTO_GIT_COMMIT_SHORT} (${OPENAUTO_GIT_COMMIT_FULL})")
message(STATUS "  Describe: ${OPENAUTO_GIT_DESCRIBE}")
message(STATUS "  Build Date: ${OPENAUTO_BUILD_DATE}")

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
option(ENABLE_REST_API "Enable REST API server" ON)
option(ENABLE_EVENT_BUS "Enable event bus system" ON)
option(ENABLE_STATE_MACHINE "Enable state machine system" ON)
option(ENABLE_SANITIZERS "Enable AddressSanitizer and UBSan" OFF)

if(ENABLE_MODERN_API)
    message(STATUS "Modern API enabled")
    
    # Find required packages for modern components
    find_package(PkgConfig REQUIRED)
    find_package(NlohmannJson REQUIRED)
    find_package(Httplib REQUIRED)
    find_package(Threads REQUIRED)
    
    if(NOT NlohmannJson_FOUND)
        message(FATAL_ERROR "nlohmann/json not found. Please install nlohmann-json-dev package.")
    endif()
    
    if(NOT Httplib_FOUND)
        message(FATAL_ERROR "cpp-httplib not found. Please install cpp-httplib or download httplib.h manually.")
    endif()
    
    message(STATUS "Found nlohmann/json: ${NLOHMANN_JSON_VERSION}")
    message(STATUS "Found cpp-httplib: ${HTTPLIB_VERSION}")
    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DENABLE_MODERN_API")
    
    # Modern sources
    set(modern_sources_directory ${sources_directory}/modern)
    set(modern_include_directory ${include_directory}/modern)
    
    file(GLOB_RECURSE modern_source_files 
        ${modern_sources_directory}/*.cpp 
        ${modern_include_directory}/*.hpp)
    
    # Add modern components to autoapp
    target_sources(autoapp PRIVATE ${modern_source_files})
    target_link_libraries(autoapp PUBLIC 
        nlohmann_json::nlohmann_json 
        httplib::httplib 
        Threads::Threads)
    
    # Add modern components to btservice (for logger support)
    target_sources(btservice PRIVATE ${modern_source_files})
    target_link_libraries(btservice PUBLIC 
        nlohmann_json::nlohmann_json 
        httplib::httplib 
        Threads::Threads)
    
    # Add filesystem support for modern logger
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
        target_link_libraries(autoapp PUBLIC stdc++fs)
        target_link_libraries(btservice PUBLIC stdc++fs)
    endif()
    

    
    message(STATUS "Modern API components added to build with OpenAPI support")
endif()

# Sanitizer support
if(ENABLE_SANITIZERS)
    message(STATUS "Enabling AddressSanitizer and UBSan")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer -g")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -fsanitize=undefined")
endif()

# Additional compiler definitions based on options
if(ENABLE_REST_API)
    add_definitions(-DENABLE_REST_API)
    message(STATUS "REST API support enabled")
endif()

if(ENABLE_EVENT_BUS)
    add_definitions(-DENABLE_EVENT_BUS)
    message(STATUS "Event Bus support enabled")
endif()

if(ENABLE_STATE_MACHINE)
    add_definitions(-DENABLE_STATE_MACHINE)
    message(STATUS "State Machine support enabled")
endif()

# Testing support
option(BUILD_TESTS "Build tests" OFF)
enable_testing()

# Testing dependencies
if(BUILD_TESTS)
    # GoogleTest
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
    
    # Google Mock
    find_package(GMock QUIET)
    if(NOT GMock_FOUND)
        set(gmock_build_tests OFF CACHE BOOL "" FORCE)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()
    
    # Testing include directories
    include_directories(${gtest_SOURCE_DIR}/include)
    include_directories(${gmock_SOURCE_DIR}/include)
endif()

# Test targets
if(BUILD_TESTS)
    # Unit tests
    add_subdirectory(tests)
    
    # Add custom test target
    add_custom_target(check
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running all tests"
    )
    
    # Coverage target (optional)
    option(ENABLE_COVERAGE "Enable test coverage" OFF)
    if(ENABLE_COVERAGE)
        include(CodeCoverage)
        append_coverage_compiler_flags()
        setup_target_for_coverage_gcovr_html(
            NAME coverage
            EXECUTABLE ctest
            DEPENDENCIES check
        )
    endif()
endif()

# =============================================================================
# DEBIAN PACKAGE CONFIGURATION
# =============================================================================

# Determine package name and description based on build type
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_TYPE_SUFFIX "-debug")
    set(PACKAGE_DESCRIPTION_SUFFIX " (Debug Build)")
    set(PACKAGE_DESCRIPTION_DETAIL "This is a debug build with verbose logging, debug symbols, and development tools enabled. Suitable for development and troubleshooting.")
    set(SERVICE_SUFFIX "-debug")
else()
    set(BUILD_TYPE_SUFFIX "")
    set(PACKAGE_DESCRIPTION_SUFFIX "")
    set(PACKAGE_DESCRIPTION_DETAIL "This is an optimized production build suitable for automotive deployment.")
    set(SERVICE_SUFFIX "")
endif()

# Determine architecture suffix
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ARCH_SUFFIX "-arm64")
    else()
        set(ARCH_SUFFIX "-armhf")
    endif()
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64")
    set(ARCH_SUFFIX "-amd64")
else()
    set(ARCH_SUFFIX "-${CMAKE_SYSTEM_PROCESSOR}")
endif()

# Determine platform suffix based on NOPI setting
if (NOPI STREQUAL "FALSE" OR NOPI STREQUAL "OFF")
    set(PLATFORM_SUFFIX "-pi")
endif()

# Combine all suffixes
set(PACKAGE_SUFFIX "${BUILD_TYPE_SUFFIX}${PLATFORM_SUFFIX}${ARCH_SUFFIX}")

# Package metadata
set(CPACK_PACKAGE_NAME "openauto-modern${PACKAGE_SUFFIX}")
set(CPACK_PACKAGE_VENDOR "OpenCarDev Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Modern Android Auto implementation for automotive systems${PACKAGE_DESCRIPTION_SUFFIX}")

# Create platform-specific description
if (NOPI)
    set(PLATFORM_DESCRIPTION "Built for non-Raspberry Pi systems (x86/x64 platforms).")
else()
    set(PLATFORM_DESCRIPTION "Built for Raspberry Pi 3 and lower.")
endif()

set(CPACK_PACKAGE_DESCRIPTION "OpenAuto is a modern Android Auto implementation that allows you to use Android Auto on various automotive systems. This package includes the modernized architecture with REST API, event bus, state machine, and comprehensive logging. ${PACKAGE_DESCRIPTION_DETAIL} ${PLATFORM_DESCRIPTION} Built from commit ${OPENAUTO_GIT_COMMIT_SHORT} on branch ${OPENAUTO_GIT_BRANCH}.")
set(CPACK_PACKAGE_VERSION_MAJOR ${OPENAUTO_BUILD_MAJOR_RELEASE})
set(CPACK_PACKAGE_VERSION_MINOR ${OPENAUTO_BUILD_MINOR_RELEASE})
set(CPACK_PACKAGE_VERSION_PATCH ${OPENAUTO_BUILD_INCREMENTAL})
set(CPACK_PACKAGE_VERSION "${PROGRAM_VERSION_STRING}")
set(CPACK_PACKAGE_CONTACT "support@opencardev.com")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/opencardev/openauto")

# Debian-specific settings
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "OpenCarDev Team <support@opencardev.com>")
set(CPACK_DEBIAN_PACKAGE_SECTION "electronics")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

# Auto-detect architecture
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
    else()
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "armhf")
    endif()
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
else()
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "all")
endif()

# Dependencies (different for debug vs release)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Debug packages include development tools and debugging libraries
    set(CPACK_DEBIAN_PACKAGE_DEPENDS 
        "libboost-all-dev (>= 1.81), libprotobuf-dev (>= 3.6), libssl-dev (>= 1.1), libusb-1.0-0-dev (>= 1.0), libudev-dev (>= 237), qtbase5-dev (>= 5.11), qtmultimedia5-dev (>= 5.11), qtconnectivity5-dev (>= 5.11), libasound2-dev (>= 1.1), libpulse-dev (>= 12.0), librtaudio-dev (>= 5.0), nlohmann-json3-dev (>= 3.6), libevent-dev (>= 2.1), systemd (>= 237), gdb, valgrind, strace, libtag1-dev (>= 1.11), libgps-dev (>= 3.20), libaasdk-dev (>= 2025.7.24)")
    
    set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "pulseaudio, bluez, bluez-tools, build-essential, cmake")
    set(CPACK_DEBIAN_PACKAGE_SUGGESTS "hostapd, dnsmasq, wireshark, tcpdump")
else()
    # Production packages only need runtime libraries
    # Use flexible versioning to support multiple library versions
    set(CPACK_DEBIAN_PACKAGE_DEPENDS 
        "libboost-system1.81.0, libboost-log1.81.0, libprotobuf32, libssl3 | libssl1.1, libusb-1.0-0 (>= 1.0), libudev1 (>= 237), qtbase5-dev (>= 5.11), qtmultimedia5-dev (>= 5.11), qtconnectivity5-dev (>= 5.11), libasound2 (>= 1.1), libpulse0 (>= 12.0), librtaudio6 | librtaudio5 (>= 5.0), systemd (>= 237), libtag1-dev (>= 1.11), libgps-dev (>= 3.20), libaasdk-dev (>= 2025.7.24)")
    
    set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "pulseaudio, bluez, bluez-tools")
    set(CPACK_DEBIAN_PACKAGE_SUGGESTS "hostapd, dnsmasq")
endif()

# Package conflicts - debug and release packages conflict with each other
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CPACK_DEBIAN_PACKAGE_CONFLICTS "openauto-modern")
    set(CPACK_DEBIAN_PACKAGE_PROVIDES "openauto-modern-debug")
else()
    set(CPACK_DEBIAN_PACKAGE_CONFLICTS "openauto-modern-debug")
    set(CPACK_DEBIAN_PACKAGE_PROVIDES "openauto-modern")
endif()

# Pre/post installation scripts (build-type specific)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA 
        "${CMAKE_SOURCE_DIR}/packaging/debian/preinst-debug;${CMAKE_SOURCE_DIR}/packaging/debian/postinst-debug;${CMAKE_SOURCE_DIR}/packaging/debian/prerm-debug;${CMAKE_SOURCE_DIR}/packaging/debian/postrm-debug")
else()
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA 
        "${CMAKE_SOURCE_DIR}/packaging/debian/preinst;${CMAKE_SOURCE_DIR}/packaging/debian/postinst;${CMAKE_SOURCE_DIR}/packaging/debian/prerm;${CMAKE_SOURCE_DIR}/packaging/debian/postrm")
endif()

# Enable CPack for package generation (must be after setting all CPACK variables)
include(CPack)

# =============================================================================
# INSTALLATION CONFIGURATION
# =============================================================================

# Set installation paths based on build type
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(INSTALL_BIN_DIR "/opt/openauto-debug")
    set(INSTALL_CONFIG_DIR "/etc/openauto-debug")
    set(INSTALL_DATA_DIR "/var/lib/openauto-debug")
    set(INSTALL_LOG_DIR "/var/log/openauto-debug")
    set(INSTALL_DOC_DIR "/usr/share/doc/openauto-modern-debug")
else()
    set(INSTALL_BIN_DIR "/opt/openauto")
    set(INSTALL_CONFIG_DIR "/etc/openauto")
    set(INSTALL_DATA_DIR "/var/lib/openauto")
    set(INSTALL_LOG_DIR "/var/log/openauto")
    set(INSTALL_DOC_DIR "/usr/share/doc/openauto-modern")
endif()

# Install binaries
install(TARGETS autoapp 
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

install(TARGETS btservice 
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

# Install configuration files (build-type specific templates)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    install(FILES packaging/config/openauto-debug.conf
        DESTINATION ${INSTALL_CONFIG_DIR}
        RENAME openauto.conf
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
    
    install(FILES packaging/config/logger-debug.conf
        DESTINATION ${INSTALL_CONFIG_DIR}
        RENAME logger.conf
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
    
    install(FILES packaging/config/services-debug.conf
        DESTINATION ${INSTALL_CONFIG_DIR}
        RENAME services.conf
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
else()
    install(FILES packaging/config/openauto.conf
        DESTINATION ${INSTALL_CONFIG_DIR}
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

    install(FILES packaging/config/logger.conf
        DESTINATION ${INSTALL_CONFIG_DIR}
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

    install(FILES packaging/config/services.conf
        DESTINATION ${INSTALL_CONFIG_DIR}
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
endif()

#Install default legacy ini configuration files
install(FILES packaging/config/openauto.ini
    DESTINATION ${INSTALL_CONFIG_DIR}
    RENAME openauto.ini
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
install(FILES packaging/config/openauto-logs.ini
    DESTINATION ${INSTALL_CONFIG_DIR}
    RENAME openauto-logs.ini
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

# Install systemd service files (build-type specific)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    install(FILES packaging/systemd/openauto-debug.service
        DESTINATION /etc/systemd/system
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

    install(FILES packaging/systemd/openauto-btservice-debug.service
        DESTINATION /etc/systemd/system
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
else()
    install(FILES packaging/systemd/openauto.service
        DESTINATION /etc/systemd/system
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

    install(FILES packaging/systemd/openauto-btservice.service
        DESTINATION /etc/systemd/system
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
endif()

# Install udev rules (shared between debug and release)
install(FILES packaging/udev/99-openauto.rules
    DESTINATION /etc/udev/rules.d
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

# Install scripts
install(FILES packaging/scripts/openauto-setup.sh
    DESTINATION ${INSTALL_BIN_DIR}/scripts
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

install(FILES packaging/scripts/openauto-monitor.sh
    DESTINATION ${INSTALL_BIN_DIR}/scripts
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

# Install documentation
install(FILES README.md RELEASE.txt LICENSE
    DESTINATION ${INSTALL_DOC_DIR})

install(DIRECTORY docs/
    DESTINATION ${INSTALL_DOC_DIR}/docs
    FILES_MATCHING PATTERN "*.md")

# Install default data directory structure
install(DIRECTORY DESTINATION ${INSTALL_DATA_DIR}
    DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE)

install(DIRECTORY DESTINATION ${INSTALL_LOG_DIR}
    DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE)

# Install logrotate configuration (build-type specific)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    install(FILES packaging/logrotate/openauto-debug
        DESTINATION /etc/logrotate.d
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
else()
    install(FILES packaging/logrotate/openauto
        DESTINATION /etc/logrotate.d
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
endif()