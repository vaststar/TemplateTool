# =============================================================================
# FFmpeg Libs - Windows Platform
# =============================================================================
# Import libs (*.lib) live in lib/, runtime DLLs (versioned, e.g. avcodec-61.dll)
# live in bin/.

# Architecture directory: x64 or arm64.
if(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64|arm64|aarch64")
    set(FFLIB_ARCH_DIR "arm64")
else()
    set(FFLIB_ARCH_DIR "x64")
endif()

set(FFLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${FFLIB_ARCH_DIR}")
set(FFLIB_INCLUDE "${FFLIB_ROOT}/include")
set(FFLIB_LIBDIR "${FFLIB_ROOT}/lib")
set(FFLIB_BINDIR "${FFLIB_ROOT}/bin")

# Skeleton-safe: skip (do not define ffmpeg_lib) until binaries are committed.
if(NOT EXISTS "${FFLIB_INCLUDE}/libavcodec/avcodec.h")
    message(STATUS "[FFmpegLibs] Windows ${FFLIB_ARCH_DIR}: binaries not provisioned yet, skipping (run build-ffmpeg-libs workflow)")
    return()
endif()

# Core import libraries required for capture/encode/mux pipelines.
set(FFLIB_CORE avformat avcodec swscale swresample avutil)
set(FFLIB_TARGETS "")
foreach(name IN LISTS FFLIB_CORE)
    find_library(FFLIB_${name}
        NAMES ${name}
        PATHS "${FFLIB_LIBDIR}"
        NO_DEFAULT_PATH
    )
    if(NOT FFLIB_${name})
        message(FATAL_ERROR "[FFmpegLibs] Missing ${name}.lib in ${FFLIB_LIBDIR}")
    endif()
    list(APPEND FFLIB_TARGETS "${FFLIB_${name}}")
endforeach()

file(GLOB FFLIB_DLLS "${FFLIB_BINDIR}/*.dll")
if(NOT FFLIB_DLLS)
    message(FATAL_ERROR "[FFmpegLibs] Runtime DLLs not found in ${FFLIB_BINDIR}")
endif()

message(STATUS "[FFmpegLibs] Windows ${FFLIB_ARCH_DIR}")
message(STATUS "[FFmpegLibs] Include: ${FFLIB_INCLUDE}")
message(STATUS "[FFmpegLibs] Libs: ${FFLIB_TARGETS}")

# Copy runtime DLLs next to the executables (per-config output dir).
add_custom_target(copy_ffmpeg_lib)
set_target_properties(copy_ffmpeg_lib PROPERTIES FOLDER framework/thirdparty/ffmpeg)
add_custom_command(TARGET copy_ffmpeg_lib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FFLIB_DLLS} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/"
    COMMAND_EXPAND_LISTS
    COMMENT "[FFmpegLibs] Copying DLLs to output directory"
)

add_library(ffmpeg_lib INTERFACE)
target_include_directories(ffmpeg_lib INTERFACE "${FFLIB_INCLUDE}")
target_link_libraries(ffmpeg_lib INTERFACE ${FFLIB_TARGETS})
add_dependencies(ffmpeg_lib copy_ffmpeg_lib)

install(FILES ${FFLIB_DLLS} DESTINATION bin)
