cmake_minimum_required(VERSION 3.5.1)

# Integration test sources
set(INTEGRATION_TEST_SOURCES
    main.cpp
    test_android_auto_integration.cpp
    test_bluetooth_integration.cpp
    test_ui_integration.cpp
    test_api_integration.cpp
    test_configuration_integration.cpp
    test_end_to_end.cpp
)

# Create integration test executable
add_executable(integration_tests ${INTEGRATION_TEST_SOURCES})

# Link libraries
target_link_libraries(integration_tests PRIVATE
    ${TEST_LIBRARIES}
    # Link against actual binaries for integration testing
)

# Set test properties
set_target_properties(integration_tests PROPERTIES
    CXX_STANDARD 17
    COMPILE_FLAGS "-DENABLE_MODERN_API -DINTEGRATION_TESTING")

# Add to CTest
add_test(NAME IntegrationTests COMMAND integration_tests)
set_tests_properties(IntegrationTests PROPERTIES 
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    TIMEOUT 600  # Longer timeout for integration tests
)
