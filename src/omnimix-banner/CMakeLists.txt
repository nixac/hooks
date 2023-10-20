##    omnimix_banner.shared    ##
set(target_name "omnimix_banner")
file(GLOB_RECURSE FILE_${target_name} CONFIGURE_DEPENDS "*.c")
add_library(${target_name} SHARED ${FILE_${target_name}})

# Local #define
target_compile_definitions(${target_name} PRIVATE
        BIN_BUILDREV="${CMAKE_BuildRevision}"
        )

# Local includes (#include "" | gcc -iquote)
target_include_directories_quote(${target_name} PRIVATE
        .
        Style
        )

# External includes (#include <> | gcc -I)
target_include_directories(${target_name} PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/mihook/include
        )

# Force includes (include for every file | gcc -include)
target_force_include(${target_name} QUOTE
        )

# Local dynamic linker (compiled libraries) - referenced libraries
target_link_libraries(${target_name} PRIVATE
        minhook
        )

# External dynamic linker (binary libraries) - external/system libraries
target_link_libraries(${target_name} PRIVATE
        )

# Post build hooks
target_init_vsmacro(${target_name})
execute_process(
        COMMAND python "${SolutionDir}lib/exec_u001/main.py" "--root=${SolutionDir}" "--exiv=${ProjectDir}on_init.exiv" "-m:ProjectDir=${ProjectDir}" "-m:TargetDir=${TargetDir}" "-m:PlatformTarget=${PlatformTarget}" "-m:PlatformLib=nix_gcc"
        )
add_custom_command(TARGET ${target_name}
        POST_BUILD
        COMMAND python "${SolutionDir}lib/exec_u001/main.py" "--root=${SolutionDir}" "--exiv=${ProjectDir}on_compiled.exiv" "-m:ProjectDir=${ProjectDir}" "-m:TargetDir=${TargetDir}" "-m:PlatformTarget=${PlatformTarget}" "-m:PlatformLib=nix_gcc"
        )
