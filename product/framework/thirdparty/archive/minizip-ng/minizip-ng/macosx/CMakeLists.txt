# =============================================================================
# minizip-ng - macOS Platform (Prebuilt)
# =============================================================================
# Prebuilt with DEFLATE compression (zlib statically linked into dylib)
# Uses dynamic library (.dylib) for linking
# =============================================================================

# Detect architecture
if(CMAKE_OSX_ARCHITECTURES MATCHES "arm64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    set(MZ_ARCH_DIR "arm")
else()
    set(MZ_ARCH_DIR "intel")
endif()

set(MZ_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${MZ_ARCH_DIR}")

# Verify prebuilt exists (headers are in minizip-ng subdirectory)
if(NOT EXISTS "${MZ_ROOT}/include/minizip-ng/mz.h")
    message(FATAL_ERROR "[minizip] Prebuilt not found at ${MZ_ROOT}. Run GitHub Action 'build-minizip-all-platforms.yml' first.")
endif()

# Find dynamic library (.dylib)
find_library(MZ_LIBRARY
    NAMES minizip-ng
    PATHS "${MZ_ROOT}/lib"
    NO_DEFAULT_PATH
)

if(NOT MZ_LIBRARY)
    message(FATAL_ERROR "[minizip] Dynamic library not found in ${MZ_ROOT}/lib")
endif()

# Find all dylibs for copying
file(GLOB MZ_DYLIBS "${MZ_ROOT}/lib/*.dylib")

message(STATUS "[minizip] macOS ${MZ_ARCH_DIR}")
message(STATUS "[minizip] Dynamic Library: ${MZ_LIBRARY}")
message(STATUS "[minizip] Dylibs: ${MZ_DYLIBS}")

# Create copy target for dylibs
if(MZ_DYLIBS)
    add_custom_target(copy_minizip)
    set_target_properties(copy_minizip PROPERTIES FOLDER framework/thirdparty/archive)

    add_custom_command(TARGET copy_minizip POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MZ_DYLIBS} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
        COMMAND_EXPAND_LISTS
        COMMENT "[minizip] Copying dylibs to output directory"
    )
endif()

# Create interface library
add_library(minizip INTERFACE)
target_include_directories(minizip INTERFACE "${MZ_ROOT}/include/minizip-ng")
target_link_libraries(minizip INTERFACE "${MZ_LIBRARY}")

if(TARGET copy_minizip)
    add_dependencies(minizip copy_minizip)
endif()

# Alias for unified access
add_library(archive::zip ALIAS minizip)

# Install dylibs to installation directory
if(MZ_DYLIBS)
    install(FILES ${MZ_DYLIBS} DESTINATION bin)
endif()
