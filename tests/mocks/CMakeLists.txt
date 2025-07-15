cmake_minimum_required(VERSION 3.5.1)

# Mock objects for testing
set(MOCK_SOURCES
    MockEventBus.hpp
    MockConfigurationManager.hpp
    MockStateMachine.hpp
    MockAndroidAutoEntity.hpp
    MockBluetoothHandler.hpp
    MockNetworkManager.hpp
)

# Create mock interface library
add_library(test_mocks INTERFACE)

# Set include directories for mocks
target_include_directories(test_mocks INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link with testing libraries
target_link_libraries(test_mocks INTERFACE
    gtest
    gmock
)
