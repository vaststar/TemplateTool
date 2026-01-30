# =============================================================================
# Compression Library - zlib-ng
# =============================================================================
# High-performance zlib replacement with DEFLATE/gzip support
# Source: https://github.com/zlib-ng/zlib-ng
# Version: 2.2.4
# =============================================================================

# Configure zlib-ng build options
set(ZLIB_COMPAT ON CACHE BOOL "Enable zlib-compatible API" FORCE)
set(ZLIB_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(ZLIBNG_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(WITH_GZFILEOP ON CACHE BOOL "Enable gzopen/gzwrite API" FORCE)
set(WITH_OPTIM ON CACHE BOOL "Enable SIMD optimizations" FORCE)
set(WITH_NEW_STRATEGIES ON CACHE BOOL "" FORCE)
set(WITH_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(WITH_FUZZERS OFF CACHE BOOL "" FORCE)

# Build static library only
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Add official zlib-ng CMake
add_subdirectory(zlib-ng zlib-ng_build)

# Create interface library to wrap zlib-ng
# Cannot create ALIAS of ALIAS, so we use INTERFACE library
add_library(compression_gzip_impl INTERFACE)
target_link_libraries(compression_gzip_impl INTERFACE zlib-ng)
add_library(compression::gzip ALIAS compression_gzip_impl)

message(STATUS "=== Compression Configuration ===")
message(STATUS "  Backend: zlib-ng (DEFLATE/gzip)")
message(STATUS "=================================")
