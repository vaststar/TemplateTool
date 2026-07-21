# =============================================================================
# FFmpeg Libs - Linux Platform
# =============================================================================

set(FFLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/x64")
set(FFLIB_INCLUDE "${FFLIB_ROOT}/include")
set(FFLIB_LIBDIR "${FFLIB_ROOT}/lib")

# Skeleton-safe: skip (do not define ffmpeg_lib) until binaries are committed.
if(NOT EXISTS "${FFLIB_INCLUDE}/libavcodec/avcodec.h")
    message(STATUS "[FFmpegLibs] Linux x64: binaries not provisioned yet, skipping (run build-ffmpeg-libs workflow)")
    return()
endif()

# Core libraries required for capture/encode/mux pipelines.
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

file(GLOB FFLIB_SHARED "${FFLIB_LIBDIR}/*.so*")

message(STATUS "[FFmpegLibs] Linux x64")
message(STATUS "[FFmpegLibs] Include: ${FFLIB_INCLUDE}")
message(STATUS "[FFmpegLibs] Libs: ${FFLIB_TARGETS}")

# Copy runtime shared objects (and version symlinks) next to the executables.
if(FFLIB_SHARED)
    add_custom_target(copy_ffmpeg_lib)
    set_target_properties(copy_ffmpeg_lib PROPERTIES FOLDER framework/thirdparty/ffmpeg)
    add_custom_command(TARGET copy_ffmpeg_lib POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FFLIB_SHARED} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
        COMMAND_EXPAND_LISTS
        COMMENT "[FFmpegLibs] Copying shared libraries to output directory"
    )
endif()

add_library(ffmpeg_lib INTERFACE)
target_include_directories(ffmpeg_lib INTERFACE "${FFLIB_INCLUDE}")
target_link_libraries(ffmpeg_lib INTERFACE ${FFLIB_TARGETS})

if(TARGET copy_ffmpeg_lib)
    add_dependencies(ffmpeg_lib copy_ffmpeg_lib)
endif()

if(FFLIB_SHARED)
    install(FILES ${FFLIB_SHARED} DESTINATION bin)
endif()
