# =============================================================================
# FFmpeg Libs - macOS Platform
# =============================================================================

include(DetectPlatform)
detect_platform(PLATFORM_TYPE)

if(PLATFORM_TYPE STREQUAL "MAC_ARM")
    set(FFLIB_ARCH_DIR "arm")
elseif(PLATFORM_TYPE STREQUAL "MAC_INTEL")
    set(FFLIB_ARCH_DIR "intel")
else()
    message(FATAL_ERROR "[FFmpegLibs] Unsupported macOS architecture: ${PLATFORM_TYPE}")
endif()

set(FFLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${FFLIB_ARCH_DIR}")
set(FFLIB_INCLUDE "${FFLIB_ROOT}/include")
set(FFLIB_LIBDIR "${FFLIB_ROOT}/lib")

# Skeleton-safe: skip (do not define ffmpeg_lib) until binaries are committed.
if(NOT EXISTS "${FFLIB_INCLUDE}/libavcodec/avcodec.h")
    message(STATUS "[FFmpegLibs] macOS ${FFLIB_ARCH_DIR}: binaries not provisioned yet, skipping (run build-ffmpeg-libs workflow)")
    return()
endif()

# Core libraries required for capture/encode/mux pipelines.
# Order matters for static resolution; harmless for shared.
set(FFLIB_CORE avformat avcodec swscale swresample avutil)
set(FFLIB_TARGETS "")
foreach(name IN LISTS FFLIB_CORE)
    find_library(FFLIB_${name}
        NAMES ${name}
        PATHS "${FFLIB_LIBDIR}"
        NO_DEFAULT_PATH
    )
    if(NOT FFLIB_${name})
        message(FATAL_ERROR "[FFmpegLibs] Missing lib${name} in ${FFLIB_LIBDIR}")
    endif()
    list(APPEND FFLIB_TARGETS "${FFLIB_${name}}")
endforeach()

file(GLOB FFLIB_DYLIBS "${FFLIB_LIBDIR}/*.dylib")

message(STATUS "[FFmpegLibs] macOS ${FFLIB_ARCH_DIR}")
message(STATUS "[FFmpegLibs] Include: ${FFLIB_INCLUDE}")
message(STATUS "[FFmpegLibs] Libs: ${FFLIB_TARGETS}")

# Copy runtime dylibs next to the executables.
if(FFLIB_DYLIBS)
    add_custom_target(copy_ffmpeg_lib)
    set_target_properties(copy_ffmpeg_lib PROPERTIES FOLDER framework/thirdparty/ffmpeg)
    add_custom_command(TARGET copy_ffmpeg_lib POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FFLIB_DYLIBS} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
        COMMAND_EXPAND_LISTS
        COMMENT "[FFmpegLibs] Copying dylibs to output directory"
    )
endif()

add_library(ffmpeg_lib INTERFACE)
target_include_directories(ffmpeg_lib INTERFACE "${FFLIB_INCLUDE}")
target_link_libraries(ffmpeg_lib INTERFACE ${FFLIB_TARGETS})

if(TARGET copy_ffmpeg_lib)
    add_dependencies(ffmpeg_lib copy_ffmpeg_lib)
endif()

if(FFLIB_DYLIBS)
    install(FILES ${FFLIB_DYLIBS} DESTINATION bin)
endif()
