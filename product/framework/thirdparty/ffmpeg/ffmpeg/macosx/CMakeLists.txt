# =============================================================================
# FFmpeg - macOS Platform
# =============================================================================

include(DetectPlatform)
detect_platform(PLATFORM_TYPE)

# Determine architecture directory
if(PLATFORM_TYPE STREQUAL "MAC_ARM")
    set(FFMPEG_ARCH_DIR "arm")
elseif(PLATFORM_TYPE STREQUAL "MAC_INTEL")
    set(FFMPEG_ARCH_DIR "intel")
else()
    message(FATAL_ERROR "[FFmpeg] Unsupported macOS architecture: ${PLATFORM_TYPE}")
endif()

set(FFMPEG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${FFMPEG_ARCH_DIR}")

# Find executables
set(FFMPEG_EXECUTABLE "${FFMPEG_ROOT}/ffmpeg")
set(FFPROBE_EXECUTABLE "${FFMPEG_ROOT}/ffprobe")

if(NOT EXISTS "${FFMPEG_EXECUTABLE}")
    message(FATAL_ERROR "[FFmpeg] ffmpeg not found in ${FFMPEG_ROOT}")
endif()

if(NOT EXISTS "${FFPROBE_EXECUTABLE}")
    message(FATAL_ERROR "[FFmpeg] ffprobe not found in ${FFMPEG_ROOT}")
endif()

message(STATUS "[FFmpeg] macOS ${FFMPEG_ARCH_DIR}")
message(STATUS "[FFmpeg] ffmpeg: ${FFMPEG_EXECUTABLE}")
message(STATUS "[FFmpeg] ffprobe: ${FFPROBE_EXECUTABLE}")

# Create copy target to output directory
add_custom_target(copy_ffmpeg ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FFMPEG_EXECUTABLE}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FFPROBE_EXECUTABLE}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
    COMMENT "[FFmpeg] Copying executables to output directory"
)
set_target_properties(copy_ffmpeg PROPERTIES FOLDER framework/thirdparty/ffmpeg)

# Export paths for use by other targets
set(FFMPEG_EXECUTABLE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ffmpeg" CACHE INTERNAL "Path to ffmpeg executable")
set(FFPROBE_EXECUTABLE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ffprobe" CACHE INTERNAL "Path to ffprobe executable")
