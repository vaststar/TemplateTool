# ============================================================================
# Master switch: build this layer's utility modules as shared (DLL) or static (LIB).
#   UCF_BUILD_UTILITIES_SHARED (option) -> UTILITY_LIB_TYPE (library type)
# Each module appends ${UTILITY_LIB_TYPE} in BuildModule(...) and passes its own
# per-module <MODULE>_SHARED / <MODULE>_STATIC export define.
# ============================================================================
option(UCF_BUILD_UTILITIES_SHARED "Build UCF utility modules as shared libraries (DLL). OFF builds them as static libraries." ON)

if (UCF_BUILD_UTILITIES_SHARED)
    set(UTILITY_LIB_TYPE SHARED_LIB)   # -> BuildModule SHARED
else()
    set(UTILITY_LIB_TYPE STATIC_LIB)   # -> BuildModule STATIC
endif()

add_subdirectory(foundation)
add_subdirectory(extended)
