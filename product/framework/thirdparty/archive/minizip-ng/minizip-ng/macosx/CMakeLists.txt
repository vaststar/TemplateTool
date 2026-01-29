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

message(STATUS "[minizip] macOS ${MZ_ARCH_DIR}")
message(STATUS "[minizip] Dynamic Library: ${MZ_LIBRARY}")

# Create interface library
add_library(minizip INTERFACE)
target_include_directories(minizip INTERFACE "${MZ_ROOT}/include/minizip-ng")
target_link_libraries(minizip INTERFACE "${MZ_LIBRARY}")

# Alias for unified access
add_library(archive::zip ALIAS minizip)
