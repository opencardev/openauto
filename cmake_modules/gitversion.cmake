# cmake/gitversion.cmake
cmake_minimum_required(VERSION 3.0.0)

message(STATUS "Resolving GIT Version")

set(_build_version "unknown")
set(_build_branch "unknown")
set(_build_describe "unknown")
set(_commit_timestamp "unknown")
set(_build_changes "")


# Prefer environment variables if set (for Docker/Podman builds)
if(DEFINED ENV{GIT_COMMIT_ID} AND NOT "$ENV{GIT_COMMIT_ID}" STREQUAL "unknown")
  set(_build_version "$ENV{GIT_COMMIT_ID}")
  message(STATUS "GIT_COMMIT_ID from ENV: $ENV{GIT_COMMIT_ID}")
else()
  message(STATUS "GIT_COMMIT_ID not set in ENV, will use git command if possible.")
endif()
if(DEFINED ENV{GIT_BRANCH} AND NOT "$ENV{GIT_BRANCH}" STREQUAL "unknown")
  set(_build_branch "$ENV{GIT_BRANCH}")
  message(STATUS "GIT_BRANCH from ENV: $ENV{GIT_BRANCH}")
else()
  message(STATUS "GIT_BRANCH not set in ENV, will use git command if possible.")
endif()
if(DEFINED ENV{GIT_DESCRIBE} AND NOT "$ENV{GIT_DESCRIBE}" STREQUAL "unknown")
  set(_build_describe "$ENV{GIT_DESCRIBE}")
  message(STATUS "GIT_DESCRIBE from ENV: $ENV{GIT_DESCRIBE}")
else()
  message(STATUS "GIT_DESCRIBE not set in ENV, will use git command if possible.")
endif()
if(DEFINED ENV{GIT_DIRTY} AND NOT "$ENV{GIT_DIRTY}" STREQUAL "unknown")
  if($ENV{GIT_DIRTY} GREATER 0)
    set(_build_changes "*")
  else()
    set(_build_changes "")
  endif()
  message(STATUS "GIT_DIRTY from ENV: $ENV{GIT_DIRTY}")
else()
  message(STATUS "GIT_DIRTY not set in ENV, will use git command if possible.")
endif()
if(DEFINED ENV{GIT_COMMIT_TIMESTAMP} AND NOT "$ENV{GIT_COMMIT_TIMESTAMP}" STREQUAL "unknown")
  set(_commit_timestamp "$ENV{GIT_COMMIT_TIMESTAMP}")
  message(STATUS "GIT_COMMIT_TIMESTAMP from ENV: $ENV{GIT_COMMIT_TIMESTAMP}")
else()
  message(STATUS "GIT_COMMIT_TIMESTAMP not set in ENV, will use git command if possible.")
endif()

# Fallback to git commands if env vars not set
if(_build_branch STREQUAL "unknown")
  find_package(Git)
  if(GIT_FOUND)
    if(_build_version STREQUAL "unknown")
      execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY "${local_dir}"
        OUTPUT_VARIABLE _build_version
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
      )
      message(STATUS "GIT_COMMIT_ID from git command: ${_build_version}")
    endif()
    if(_build_branch STREQUAL "unknown")
      execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY "${local_dir}"
        OUTPUT_VARIABLE _build_branch
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
      )
      message(STATUS "GIT_BRANCH from git command: ${_build_branch}")
    endif()
    if(_build_describe STREQUAL "unknown")
      execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --dirty --always
        WORKING_DIRECTORY "${local_dir}"
        OUTPUT_VARIABLE _build_describe
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
      )
      message(STATUS "GIT_DESCRIBE from git command: ${_build_describe}")
    endif()
    if(_commit_timestamp STREQUAL "unknown")
      execute_process(
        COMMAND ${GIT_EXECUTABLE} log -1 --format=%at
        WORKING_DIRECTORY "${local_dir}"
        OUTPUT_VARIABLE _commit_timestamp
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
      )
      message(STATUS "GIT_COMMIT_TIMESTAMP from git command: ${_commit_timestamp}")
    endif()
    message(STATUS "Final GIT values: hash: ${_build_version}; branch: ${_build_branch}; describe: ${_build_describe}; changes: ${_build_changes}; Commit epoch: ${_commit_timestamp};")
    if(_build_changes STREQUAL "")
      execute_process(
        COMMAND ${GIT_EXECUTABLE} diff --no-ext-diff --quiet
        WORKING_DIRECTORY "${local_dir}"
        RESULT_VARIABLE ret
      )
      if(ret EQUAL "1")
        set(_build_changes "*")
      else()
        set(_build_changes "")
      endif()
    endif()
  else()
    message(STATUS "GIT not found")
  endif()
endif()

# Export variables with standard names for CMakeLists.txt
# Note: Using regular set() without PARENT_SCOPE because this is included, not called as a function
set(GIT_COMMIT_ID "${_build_version}")
set(GIT_BRANCH "${_build_branch}")
set(GIT_DESCRIBE "${_build_describe}")
set(GIT_COMMIT_TIMESTAMP "${_commit_timestamp}")

# Convert _build_changes to GIT_DIRTY (0 for clean, 1 for dirty)
if(_build_changes STREQUAL "*")
  set(GIT_DIRTY 1)
else()
  set(GIT_DIRTY 0)
endif()

message(STATUS "Exported GIT_COMMIT_ID: ${_build_version}")
message(STATUS "Exported GIT_BRANCH: ${_build_branch}")
message(STATUS "Exported GIT_DESCRIBE: ${_build_describe}")
message(STATUS "Exported GIT_COMMIT_TIMESTAMP: ${_commit_timestamp}")
message(STATUS "Exported GIT_DIRTY: ${GIT_DIRTY}")

