# =============================================================================
# minizip-ng - Windows Platform (Prebuilt)
# =============================================================================
# Prebuilt with DEFLATE compression (zlib statically linked into DLL)
# Uses DLL + import library for linking
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

# Find import library for DLL (in lib/ directory)
find_library(MZ_LIBRARY
    NAMES minizip-ng
    PATHS "${MZ_ROOT}/lib"
    NO_DEFAULT_PATH
)

if(NOT MZ_LIBRARY)
    message(FATAL_ERROR "[minizip] Import library not found in ${MZ_ROOT}/lib. Make sure to download the full artifact including lib/ directory.")
endif()

# Find DLLs
file(GLOB MZ_DLLS "${MZ_ROOT}/bin/*.dll")

if(NOT MZ_DLLS)
    message(FATAL_ERROR "[minizip] DLL not found in ${MZ_ROOT}/bin")
endif()

message(STATUS "[minizip] Windows ${MZ_ARCH_DIR}")
message(STATUS "[minizip] Import Library: ${MZ_LIBRARY}")
message(STATUS "[minizip] DLLs: ${MZ_DLLS}")

# Copy DLL target
add_custom_target(copy_minizip)
set_target_properties(copy_minizip PROPERTIES FOLDER framework/thirdparty/archive)

add_custom_command(TARGET copy_minizip POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MZ_DLLS} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/"
    COMMAND_EXPAND_LISTS
    COMMENT "[minizip] Copying DLLs to output directory"
)

# Create interface library
add_library(minizip INTERFACE)
target_include_directories(minizip INTERFACE "${MZ_ROOT}/include/minizip-ng")
target_link_libraries(minizip INTERFACE "${MZ_LIBRARY}")
add_dependencies(minizip copy_minizip)

# Alias for unified access
add_library(archive::zip ALIAS minizip)

# Install DLLs
install(FILES ${MZ_DLLS} DESTINATION bin)
