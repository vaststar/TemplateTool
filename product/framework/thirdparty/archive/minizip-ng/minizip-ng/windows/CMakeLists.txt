# =============================================================================
# minizip-ng - Windows Platform (Prebuilt)
# =============================================================================
# Prebuilt with DEFLATE compression (zlib statically linked)
# =============================================================================

include(DetectPlatform)
detect_platform(PLATFORM_TYPE)

if(PLATFORM_TYPE STREQUAL "WINDOWS_X64")
    set(MZ_ARCH_DIR "x64")
elseif(PLATFORM_TYPE STREQUAL "WINDOWS_X86")
    set(MZ_ARCH_DIR "x86")
elseif(PLATFORM_TYPE STREQUAL "WINDOWS_ARM64")
    set(MZ_ARCH_DIR "arm64")
else()
    message(FATAL_ERROR "[minizip] Unsupported Windows architecture: ${PLATFORM_TYPE}")
endif()

set(MZ_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${MZ_ARCH_DIR}")

# Verify prebuilt exists (headers are in minizip-ng subdirectory)
if(NOT EXISTS "${MZ_ROOT}/include/minizip-ng/mz.h")
    message(FATAL_ERROR "[minizip] Prebuilt not found at ${MZ_ROOT}. Run GitHub Action 'build-minizip-all-platforms.yml' first.")
endif()

# Find static library (in static/ directory, named minizip-ng)
find_library(MZ_LIBRARY
    NAMES minizip-ng
    PATHS "${MZ_ROOT}/static"
    NO_DEFAULT_PATH
)

if(NOT MZ_LIBRARY)
    message(FATAL_ERROR "[minizip] Library not found in ${MZ_ROOT}/static")
endif()

# Find DLLs
file(GLOB MZ_DLLS "${MZ_ROOT}/bin/*.dll")

message(STATUS "[minizip] Windows ${MZ_ARCH_DIR}")
message(STATUS "[minizip] Library: ${MZ_LIBRARY}")

# Copy DLL target
add_custom_target(copy_minizip)
set_target_properties(copy_minizip PROPERTIES FOLDER framework/thirdparty/archive)

if(MZ_DLLS)
    add_custom_command(TARGET copy_minizip POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MZ_DLLS} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/"
        COMMAND_EXPAND_LISTS
        COMMENT "[minizip] Copying DLLs to output directory"
    )
endif()

# Create interface library
add_library(minizip INTERFACE)
target_include_directories(minizip INTERFACE "${MZ_ROOT}/include/minizip-ng")
target_link_libraries(minizip INTERFACE "${MZ_LIBRARY}")
add_dependencies(minizip copy_minizip)

# Alias for unified access
add_library(archive::zip ALIAS minizip)

# Install DLLs
if(MZ_DLLS)
    install(FILES ${MZ_DLLS} DESTINATION bin)
endif()
