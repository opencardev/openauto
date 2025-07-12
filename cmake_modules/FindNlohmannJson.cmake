# FindNlohmannJson.cmake
# Find nlohmann/json header-only library
#
# This module defines:
# NLOHMANN_JSON_FOUND - True if nlohmann/json is found
# NLOHMANN_JSON_INCLUDE_DIRS - Include directories for nlohmann/json
# NLOHMANN_JSON_VERSION - Version of nlohmann/json if available

find_path(NLOHMANN_JSON_INCLUDE_DIR
    NAMES nlohmann/json.hpp
    PATHS
        /usr/include
        /usr/local/include
        /opt/homebrew/include
        ${CMAKE_PREFIX_PATH}/include
    DOC "nlohmann/json include directory"
)

if(NLOHMANN_JSON_INCLUDE_DIR)
    # Try to extract version from json.hpp
    if(EXISTS "${NLOHMANN_JSON_INCLUDE_DIR}/nlohmann/json.hpp")
        file(READ "${NLOHMANN_JSON_INCLUDE_DIR}/nlohmann/json.hpp" JSON_HPP_CONTENT)
        string(REGEX MATCH "#define NLOHMANN_JSON_VERSION_MAJOR ([0-9]+)" 
               JSON_VERSION_MAJOR_MATCH "${JSON_HPP_CONTENT}")
        string(REGEX MATCH "#define NLOHMANN_JSON_VERSION_MINOR ([0-9]+)" 
               JSON_VERSION_MINOR_MATCH "${JSON_HPP_CONTENT}")
        string(REGEX MATCH "#define NLOHMANN_JSON_VERSION_PATCH ([0-9]+)" 
               JSON_VERSION_PATCH_MATCH "${JSON_HPP_CONTENT}")
        
        if(JSON_VERSION_MAJOR_MATCH AND JSON_VERSION_MINOR_MATCH AND JSON_VERSION_PATCH_MATCH)
            set(NLOHMANN_JSON_VERSION "${CMAKE_MATCH_1}.${CMAKE_MATCH_1}.${CMAKE_MATCH_1}")
        endif()
    endif()
    
    set(NLOHMANN_JSON_INCLUDE_DIRS ${NLOHMANN_JSON_INCLUDE_DIR})
    set(NlohmannJson_FOUND TRUE)
    
    if(NOT TARGET nlohmann_json::nlohmann_json)
        add_library(nlohmann_json::nlohmann_json INTERFACE IMPORTED)
        set_target_properties(nlohmann_json::nlohmann_json PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${NLOHMANN_JSON_INCLUDE_DIR}"
        )
    endif()
else()
    set(NlohmannJson_FOUND FALSE)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NlohmannJson
    FOUND_VAR NlohmannJson_FOUND
    REQUIRED_VARS NLOHMANN_JSON_INCLUDE_DIR
    VERSION_VAR NLOHMANN_JSON_VERSION
)

mark_as_advanced(NLOHMANN_JSON_INCLUDE_DIR)
