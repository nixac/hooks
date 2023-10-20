##    Global settings    ##
# Binaries prefix
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_IMPORT_LIBRARY_PREFIX "")
# Binaries output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/${TOOLCHAIN_CONFIG}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/${TOOLCHAIN_CONFIG}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/${TOOLCHAIN_CONFIG}")
# Compilation profile specific
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif ()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti")
# Compiler stage
add_compile_definitions(CMAKE_BUILD WINDOWS_TARGET)
add_compile_options(-Wall -Wextra -Wno-attributes -Wno-pragmas -Wno-unknown-pragmas -Wno-eof-newline -Wno-deprecated-declarations -Wenum-conversion -Wno-unused-function -Wno-unused-parameter)
add_compile_options(-fPIC)
include_directories(
        lib/win_gcc/include
        lib/nix_gcc/include/${TOOLCHAIN_TARGET_ARCH}
)
# Linker stage
link_directories(
        "${PROJECT_SOURCE_DIR}/lib/win_gcc"
        "${PROJECT_SOURCE_DIR}/lib/win_gcc/${TOOLCHAIN_TARGET_ARCH}"
        "${PROJECT_SOURCE_DIR}/lib/win_gcc/${TOOLCHAIN_TARGET_ARCH}/dll"
)

##    Add projects    ##
# hooking support
add_subdirectory(
        lib/minhook
)
# omnimix banner coloring
add_subdirectory(
        src/omnimix-banner
)
