cmake_minimum_required(VERSION 3.5.1)

# Test configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use GoogleTest from FetchContent (configured in parent CMakeLists.txt)
if(NOT TARGET gtest)
    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})
endif()

# Include directories for tests
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/f1x/openauto
    ${CMAKE_SOURCE_DIR}/src
    ${gtest_SOURCE_DIR}/include
    ${gmock_SOURCE_DIR}/include
)

# Common test libraries
set(TEST_LIBRARIES
    gtest
    gtest_main
    gmock
    gmock_main
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Multimedia_LIBRARIES}
    ${Qt5MultimediaWidgets_LIBRARIES}
    ${Qt5Bluetooth_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${RTAUDIO_LIBRARIES}
    ${TAGLIB_LIBRARIES}
    ${BLKID_LIBRARIES}
    ${AAP_PROTOBUF_LIBRARIES}
    ${AASDK_LIBRARIES}
    ${BCM_HOST_LIBRARIES}
    ${ILCLIENT_LIBRARIES}
    nlohmann_json::nlohmann_json
    httplib::httplib
    Threads::Threads
)

# Include main project directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${aasdk_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${Qt5Multimedia_INCLUDE_DIRS}
    ${Qt5MultimediaWidgets_INCLUDE_DIRS}
    ${Qt5Bluetooth_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${PROTOBUF_INCLUDE_DIRS}
    ${aap_protobuf_INCLUDE_DIRS}
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
    pthread
    openauto
    ${aasdk_LIBRARIES}
    ${Boost_LIBRARIES}
    ${Qt5Multimedia_LIBRARIES}
    ${Qt5MultimediaWidgets_LIBRARIES}
    ${Qt5Bluetooth_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${PROTOBUF_LIBRARIES}
    ${LIBUSB_1_LIBRARIES}
    ${RTAUDIO_LIBRARIES}
    ${aap_protobuf_LIBRARIES}
)

target_link_libraries(integration_tests
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
    openauto
    ${aasdk_LIBRARIES}
    ${Boost_LIBRARIES}
    ${Qt5Multimedia_LIBRARIES}
    ${Qt5MultimediaWidgets_LIBRARIES}
    ${Qt5Bluetooth_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${PROTOBUF_LIBRARIES}
    ${LIBUSB_1_LIBRARIES}
    ${RTAUDIO_LIBRARIES}
    ${aap_protobuf_LIBRARIES}
)

# Unit tests subdirectories
add_subdirectory(unit)
add_subdirectory(integration)
add_subdirectory(mocks)

# Test data
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/test_data/test_config.json
    ${CMAKE_CURRENT_BINARY_DIR}/test_data/test_config.json
    COPYONLY
)

# Test resources
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_data/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/test_data/)