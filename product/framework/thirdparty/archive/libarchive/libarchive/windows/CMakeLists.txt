# =============================================================================
# libarchive - Windows Platform (Prebuilt)
# =============================================================================

include(DetectPlatform)
detect_platform(PLATFORM_TYPE)

if(PLATFORM_TYPE STREQUAL "WINDOWS_X64")
    set(LA_ARCH_DIR "x64")
elseif(PLATFORM_TYPE STREQUAL "WINDOWS_X86")
    set(LA_ARCH_DIR "x86")
elseif(PLATFORM_TYPE STREQUAL "WINDOWS_ARM64")
    set(LA_ARCH_DIR "arm64")
else()
    message(FATAL_ERROR "[libarchive] Unsupported Windows architecture: ${PLATFORM_TYPE}")
endif()

set(LA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${LA_ARCH_DIR}")

if(NOT EXISTS "${LA_ROOT}/include/archive.h")
    message(FATAL_ERROR "[libarchive] Prebuilt not found at ${LA_ROOT}. Run GitHub Action 'build-libarchive-all-platforms.yml' first.")
endif()

find_library(LA_LIBRARY
    NAMES archive libarchive
    PATHS "${LA_ROOT}/lib"
    NO_DEFAULT_PATH
)

if(NOT LA_LIBRARY)
    message(FATAL_ERROR "[libarchive] Import library not found in ${LA_ROOT}/lib")
endif()

file(GLOB LA_DLLS "${LA_ROOT}/bin/*.dll")

if(NOT LA_DLLS)
    message(FATAL_ERROR "[libarchive] DLL not found in ${LA_ROOT}/bin")
endif()

message(STATUS "[libarchive] Windows ${LA_ARCH_DIR}")
message(STATUS "[libarchive] Import Library: ${LA_LIBRARY}")
message(STATUS "[libarchive] DLLs: ${LA_DLLS}")

add_custom_target(copy_libarchive)
set_target_properties(copy_libarchive PROPERTIES FOLDER framework/thirdparty/archive)

add_custom_command(TARGET copy_libarchive POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LA_DLLS} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/"
    COMMAND_EXPAND_LISTS
    COMMENT "[libarchive] Copying DLLs to output directory"
)

add_library(libarchive_prebuilt INTERFACE)
target_include_directories(libarchive_prebuilt INTERFACE "${LA_ROOT}/include")
target_link_libraries(libarchive_prebuilt INTERFACE "${LA_LIBRARY}")
add_dependencies(libarchive_prebuilt copy_libarchive)

add_library(archive::multi ALIAS libarchive_prebuilt)

install(FILES ${LA_DLLS} DESTINATION bin)
