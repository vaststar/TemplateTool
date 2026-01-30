# =============================================================================
# minizip-ng - Linux Platform (Prebuilt)
# =============================================================================
# Prebuilt with DEFLATE compression (zlib statically linked into .so)
# Uses dynamic library (.so) for linking
# =============================================================================

set(MZ_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/x64")

# Verify prebuilt exists (headers are in minizip-ng subdirectory)
if(NOT EXISTS "${MZ_ROOT}/include/minizip-ng/mz.h")
    message(FATAL_ERROR "[minizip] Prebuilt not found at ${MZ_ROOT}. Run GitHub Action 'build-minizip-all-platforms.yml' first.")
endif()

# Find dynamic library (.so)
find_library(MZ_LIBRARY
    NAMES minizip-ng
    PATHS "${MZ_ROOT}/lib"
    NO_DEFAULT_PATH
)

if(NOT MZ_LIBRARY)
    message(FATAL_ERROR "[minizip] Dynamic library not found in ${MZ_ROOT}/lib")
endif()

# Find all .so files for copying
file(GLOB MZ_SOLIBS "${MZ_ROOT}/lib/*.so*")

message(STATUS "[minizip] Linux x64")
message(STATUS "[minizip] Dynamic Library: ${MZ_LIBRARY}")
message(STATUS "[minizip] Shared Objects: ${MZ_SOLIBS}")

# Create copy target for .so files
if(MZ_SOLIBS)
    add_custom_target(copy_minizip)
    set_target_properties(copy_minizip PROPERTIES FOLDER framework/thirdparty/archive)

    add_custom_command(TARGET copy_minizip POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MZ_SOLIBS} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
        COMMAND_EXPAND_LISTS
        COMMENT "[minizip] Copying shared objects to output directory"
    )
endif()

# Create interface library
add_library(minizip INTERFACE)
target_include_directories(minizip INTERFACE "${MZ_ROOT}/include/minizip-ng")
target_link_libraries(minizip INTERFACE "${MZ_LIBRARY}")

if(TARGET copy_minizip)
    add_dependencies(minizip copy_minizip)
endif()

# Set RPATH for runtime
set_target_properties(minizip PROPERTIES
    INTERFACE_LINK_OPTIONS "-Wl,-rpath,$ORIGIN"
)

# Alias for unified access
add_library(archive::zip ALIAS minizip)

# Install shared objects to installation directory
if(MZ_SOLIBS)
    install(FILES ${MZ_SOLIBS} DESTINATION bin)
endif()
