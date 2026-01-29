# =============================================================================
# minizip-ng - Android Platform (Prebuilt)
# =============================================================================
# Prebuilt with DEFLATE compression (zlib statically linked)
# =============================================================================

# Get Android ABI
if(NOT ANDROID_ABI)
    message(FATAL_ERROR "[minizip] ANDROID_ABI not defined")
endif()

set(MZ_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${ANDROID_ABI}")

# Verify prebuilt exists (headers are in minizip-ng subdirectory)
if(NOT EXISTS "${MZ_ROOT}/include/minizip-ng/mz.h")
    message(FATAL_ERROR "[minizip] Prebuilt not found at ${MZ_ROOT}. Run GitHub Action 'build-minizip-all-platforms.yml' first.")
endif()

# Find shared library (named minizip-ng)
find_library(MZ_LIBRARY
    NAMES minizip-ng
    PATHS "${MZ_ROOT}/lib"
    NO_DEFAULT_PATH
)

if(NOT MZ_LIBRARY)
    message(FATAL_ERROR "[minizip] Library not found in ${MZ_ROOT}/lib")
endif()

message(STATUS "[minizip] Android ${ANDROID_ABI}")
message(STATUS "[minizip] Library: ${MZ_LIBRARY}")

# Create interface library
add_library(minizip INTERFACE)
target_include_directories(minizip INTERFACE "${MZ_ROOT}/include/minizip-ng")
target_link_libraries(minizip INTERFACE "${MZ_LIBRARY}")

# Alias for unified access
add_library(archive::zip ALIAS minizip)
