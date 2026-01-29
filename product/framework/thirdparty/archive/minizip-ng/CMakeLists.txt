# =============================================================================
# Archive Library - minizip-ng
# =============================================================================
# Uses official minizip-ng repository without modification
# Source: https://github.com/zlib-ng/minizip-ng
# Version: 4.0.7
# =============================================================================

# Configure minizip-ng build options
set(MZ_COMPAT OFF CACHE BOOL "" FORCE)           # No legacy API compatibility
set(MZ_LIB_SUFFIX "" CACHE STRING "" FORCE)      # Use 'minizip' as target name (not 'minizip-ng')
set(MZ_ZLIB OFF CACHE BOOL "" FORCE)             # Disable zlib (use store mode)
set(MZ_BZIP2 OFF CACHE BOOL "" FORCE)            # Disable bzip2
set(MZ_LZMA OFF CACHE BOOL "" FORCE)             # Disable lzma
set(MZ_ZSTD OFF CACHE BOOL "" FORCE)             # Disable zstd
set(MZ_LIBCOMP OFF CACHE BOOL "" FORCE)          # Disable Apple libcompression
set(MZ_OPENSSL OFF CACHE BOOL "" FORCE)          # Disable encryption
set(MZ_BCRYPT OFF CACHE BOOL "" FORCE)           # Disable Windows bcrypt
set(MZ_FETCH_LIBS OFF CACHE BOOL "" FORCE)       # Do NOT fetch external libs
set(MZ_BUILD_TESTS OFF CACHE BOOL "" FORCE)      # No tests
set(MZ_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE) # No unit tests
set(MZ_BUILD_FUZZ_TESTS OFF CACHE BOOL "" FORCE) # No fuzz tests

# Add official minizip-ng CMake
add_subdirectory(minizip-ng)

# Create unified interface alias
add_library(archive::zip ALIAS minizip)

message(STATUS "=== Archive Configuration ===")
message(STATUS "  Backend: minizip-ng (store mode, no compression)")
message(STATUS "=============================")
