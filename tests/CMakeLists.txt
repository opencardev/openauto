cmake_minimum_required(VERSION 3.10)

# Set CMake policies to avoid warnings
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)  # Use BoostConfig.cmake instead of FindBoost.cmake
endif()

# Google Test requires at least C++11
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Locate GTest and GMock
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Add GMock support
set(GMOCK_LIBRARIES gmock gmock_main)
include_directories(/usr/include)

# Find required packages
find_package(aasdk REQUIRED)
find_package(Boost REQUIRED COMPONENTS system log program_options thread)
find_package(Qt5 REQUIRED COMPONENTS Multimedia MultimediaWidgets Bluetooth Network Test)
find_package(Protobuf REQUIRED)
find_package(libusb-1.0 REQUIRED)
find_package(rtaudio REQUIRED)
find_package(aap_protobuf REQUIRED)

# Include main project directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
    ${aasdk_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${Qt5Multimedia_INCLUDE_DIRS}
    ${Qt5MultimediaWidgets_INCLUDE_DIRS}
    ${Qt5Bluetooth_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${Qt5Test_INCLUDE_DIRS}
    ${PROTOBUF_INCLUDE_DIRS}
    ${LIBUSB_1_INCLUDE_DIRS}
    ${RTAUDIO_INCLUDE_DIRS}
)

# Add test executable
add_executable(unit_tests
    unit/main.cpp
    unit/ConnectionTests.cpp
    unit/ProjectionTests.cpp
    unit/ServiceTests.cpp
    unit/ConfigurationTests.cpp
)

add_executable(integration_tests
    integration/main.cpp
    integration/AndroidAutoIntegrationTests.cpp
    integration/BluetoothIntegrationTests.cpp
    integration/UIIntegrationTests.cpp
)

# Link test executable against Google Test and main project libraries
target_link_libraries(unit_tests
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    ${GMOCK_LIBRARIES}
    pthread
    openauto
    ${aasdk_LIBRARIES}
    ${Boost_LIBRARIES}
    ${Qt5Multimedia_LIBRARIES}
    ${Qt5MultimediaWidgets_LIBRARIES}
    ${Qt5Bluetooth_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${Qt5Test_LIBRARIES}
    ${PROTOBUF_LIBRARIES}
    ${LIBUSB_1_LIBRARIES}
    ${RTAUDIO_LIBRARIES}
    ${aap_protobuf_LIBRARIES}
)

target_link_libraries(integration_tests
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    ${GMOCK_LIBRARIES}
    pthread
    openauto
    ${aasdk_LIBRARIES}
    ${Boost_LIBRARIES}
    ${Qt5Multimedia_LIBRARIES}
    ${Qt5MultimediaWidgets_LIBRARIES}
    ${Qt5Bluetooth_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${Qt5Test_LIBRARIES}
    ${PROTOBUF_LIBRARIES}
    ${LIBUSB_1_LIBRARIES}
    ${RTAUDIO_LIBRARIES}
    ${aap_protobuf_LIBRARIES}
)

# Add tests to CTest
add_test(NAME UnitTests COMMAND unit_tests)
add_test(NAME IntegrationTests COMMAND integration_tests)