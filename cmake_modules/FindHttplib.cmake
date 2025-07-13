# FindHttplib.cmake
# Find cpp-httplib library (both header and compiled library)
#
# This module defines:
# HTTPLIB_FOUND - True if httplib is found
# HTTPLIB_INCLUDE_DIRS - Include directories for httplib
# HTTPLIB_LIBRARIES - Libraries to link against
# HTTPLIB_VERSION - Version of httplib if available

find_path(HTTPLIB_INCLUDE_DIR
    NAMES httplib.h
    PATHS
        /usr/include
        /usr/local/include
        /opt/homebrew/include
        ${CMAKE_PREFIX_PATH}/include
    DOC "httplib include directory"
)

find_library(HTTPLIB_LIBRARY
    NAMES cpp-httplib httplib
    PATHS
        /usr/lib
        /usr/local/lib
        /opt/homebrew/lib
        ${CMAKE_PREFIX_PATH}/lib
    DOC "httplib library"
)

if(HTTPLIB_INCLUDE_DIR AND HTTPLIB_LIBRARY)
    # Try to extract version from httplib.h
    if(EXISTS "${HTTPLIB_INCLUDE_DIR}/httplib.h")
        file(READ "${HTTPLIB_INCLUDE_DIR}/httplib.h" HTTPLIB_H_CONTENT)
        string(REGEX MATCH "#define HTTPLIB_VERSION \"([0-9]+\\.[0-9]+\\.[0-9]+)\"" 
               HTTPLIB_VERSION_MATCH "${HTTPLIB_H_CONTENT}")
        if(HTTPLIB_VERSION_MATCH)
            set(HTTPLIB_VERSION ${CMAKE_MATCH_1})
        endif()
    endif()
    
    set(HTTPLIB_INCLUDE_DIRS ${HTTPLIB_INCLUDE_DIR})
    set(HTTPLIB_LIBRARIES ${HTTPLIB_LIBRARY})
    set(Httplib_FOUND TRUE)
    
    if(NOT TARGET httplib::httplib)
        add_library(httplib::httplib SHARED IMPORTED)
        set_target_properties(httplib::httplib PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${HTTPLIB_INCLUDE_DIR}"
            IMPORTED_LOCATION "${HTTPLIB_LIBRARY}"
            INTERFACE_COMPILE_DEFINITIONS "CPPHTTPLIB_OPENSSL_SUPPORT;CPPHTTPLIB_ZLIB_SUPPORT;CPPHTTPLIB_BROTLI_SUPPORT"
        )
    endif()
else()
    set(Httplib_FOUND FALSE)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Httplib
    FOUND_VAR Httplib_FOUND
    REQUIRED_VARS HTTPLIB_INCLUDE_DIR HTTPLIB_LIBRARY
    VERSION_VAR HTTPLIB_VERSION
)

mark_as_advanced(HTTPLIB_INCLUDE_DIR)
