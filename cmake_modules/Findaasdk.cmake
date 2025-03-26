if (AASDK_LIB_DIRS AND AASDK_INCLUDE_DIRS)
    # in cache already
    message(STATUS "aasdk cached")
    set(AASDK_FOUND TRUE)
else (AASDK_LIB_DIRS AND AASDK_INCLUDE_DIRS)
    find_path(AASDK_INCLUDE_DIR
            NAMES
            Version.hpp
            PATHS
            /usr/include
            /usr/local/include
            /opt/local/include
            /sw/include
            PATH_SUFFIXES
            aasdk
    )

    find_library(AASDK_LIB_DIR
            NAMES
            aasdk libaasdk
            PATHS
            /usr/lib
            /usr/local/lib
            /opt/local/lib
            /sw/lib
    )

    set(AASDK_INCLUDE_DIRS
            ${AASDK_INCLUDE_DIR}
    )
    set(AASDK_LIB_DIRS
            ${AASDK_LIB_DIR}
    )

    if (AASDK_INCLUDE_DIRS AND AASDK_LIB_DIRS)
        set(AASDK_FOUND TRUE)
    endif (AASDK_INCLUDE_DIRS AND AASDK_LIB_DIRS)

    if (AASDK_FOUND)
        message(STATUS "Found aasdk:")
        message(STATUS " - Includes: ${AASDK_INCLUDE_DIRS}")
        message(STATUS " - Libraries: ${AASDK_LIB_DIRS}")
        add_library(aasdk INTERFACE)
        target_include_directories(aasdk SYSTEM INTERFACE ${AASDK_INCLUDE_DIR})
        target_link_libraries(aasdk INTERFACE ${AASDK_LIB_DIR})
    else (AASDK_FOUND)
        message(STATUS " - Includes: ${AASDK_INCLUDE_DIRS}")
        message(STATUS " - Libraries: ${AASDK_LIB_DIRS}")
        message(FATAL_ERROR "Could not find aasdk")
    endif (AASDK_FOUND)

    # show the AASDK_INCLUDE_DIRS and AASDK_LIB_DIRS variables only in the advanced view
    mark_as_advanced(AASDK_INCLUDE_DIRS AASDK_LIB_DIRS)

endif (AASDK_LIB_DIRS AND AASDK_INCLUDE_DIRS)
