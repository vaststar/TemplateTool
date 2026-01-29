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

message(STATUS "[minizip] Linux x64")
message(STATUS "[minizip] Dynamic Library: ${MZ_LIBRARY}")

# Create interface library
add_library(minizip INTERFACE)
target_include_directories(minizip INTERFACE "${MZ_ROOT}/include/minizip-ng")
target_link_libraries(minizip INTERFACE "${MZ_LIBRARY}")

# Set RPATH for runtime
set_target_properties(minizip PROPERTIES
    INTERFACE_LINK_OPTIONS "-Wl,-rpath,$ORIGIN"
)

# Alias for unified access
add_library(archive::zip ALIAS minizip)
