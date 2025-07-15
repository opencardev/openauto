cmake_minimum_required(VERSION 3.5.1)

# Unit test sources
set(UNIT_TEST_SOURCES
    main.cpp
    test_logger.cpp
    test_event_bus.cpp
    test_state_machine.cpp
    test_configuration_manager.cpp
    test_rest_api_server.cpp
    test_android_auto_entity.cpp
    test_service_factory.cpp
    test_bluetooth_handler.cpp
    test_wifi_projection.cpp
    test_vendor_extension.cpp
)

# Create unit test executable
add_executable(unit_tests ${UNIT_TEST_SOURCES})

# Link libraries
target_link_libraries(unit_tests PRIVATE
    ${TEST_LIBRARIES}
    # Add specific source files for testing without creating full target dependencies
    ${CMAKE_SOURCE_DIR}/src/modern/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/modern/Event.cpp
    ${CMAKE_SOURCE_DIR}/src/modern/EventBus.cpp
    ${CMAKE_SOURCE_DIR}/src/modern/StateMachine.cpp
    ${CMAKE_SOURCE_DIR}/src/modern/ConfigurationManager.cpp
    ${CMAKE_SOURCE_DIR}/src/modern/RestApiServer.cpp
)

# Set test properties
set_target_properties(unit_tests PROPERTIES
    CXX_STANDARD 17
    COMPILE_FLAGS "-DENABLE_MODERN_API -DTESTING_MODE")

# Add to CTest
add_test(NAME UnitTests COMMAND unit_tests)
set_tests_properties(UnitTests PROPERTIES 
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    TIMEOUT 300)
