# =============================================================================
# Compression Library - zstd
# =============================================================================
# Uses official zstd repository without modification
# Source: https://github.com/facebook/zstd
# Version: 1.5.6
# =============================================================================

# Configure zstd build options
set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(ZSTD_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(ZSTD_LEGACY_SUPPORT OFF CACHE BOOL "" FORCE)
set(ZSTD_MULTITHREAD_SUPPORT ON CACHE BOOL "" FORCE)

# Add official zstd CMake (entry point is build/cmake)
add_subdirectory(zstd/build/cmake zstd_build)

# Set IDE folder for zstd targets (only real targets, not ALIAS)
foreach(_target libzstd_static clean-all uninstall)
    if(TARGET "${_target}")
        get_target_property(_aliased "${_target}" ALIASED_TARGET)
        if(NOT _aliased)
            set_target_properties("${_target}" PROPERTIES FOLDER "thirdparty/compression/zstd")
        endif()
    endif()
endforeach()

# Create unified interface alias
add_library(compression::core ALIAS libzstd_static)

message(STATUS "=== Compression Configuration ===")
message(STATUS "  Backend: zstd")
message(STATUS "=================================")

