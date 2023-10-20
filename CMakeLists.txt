##    Global settings    ##
# Solution name and CMake version used
cmake_minimum_required(VERSION 3.16)
project(hooks-ac)
include("${CMAKE_TOOLCHAIN_FILE}")
include(lib/cmake-various/extension/PrecompiledHeader.cmake)
include(lib/cmake-various/extension/CMakeEx.cmake)
target_init_build_revision(CMAKE)
message(STATUS "BuildRevision is ${CMAKE_BuildRevision}")

##    Load main config    ##
if (CMAKE_SYSTEM_NAME MATCHES ".*[W|w]indows.*")
    include(CMakeLists_win-gcc.cmake)
else ()
    message(FATAL_ERROR "Unsupported target platform")
endif ()

##    Post-setup Info    ##
message(STATUS "Configuration name: ${TOOLCHAIN_CONFIG}")
message(STATUS "Target OS: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Target Arch: ${CMAKE_SYSTEM_PROCESSOR}")
