# =============================================================================
# FFmpeg - Windows Platform
# =============================================================================

# Windows only supports x64 for now
set(FFMPEG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/x64")

# Find executables
set(FFMPEG_EXECUTABLE "${FFMPEG_ROOT}/ffmpeg.exe")
set(FFPROBE_EXECUTABLE "${FFMPEG_ROOT}/ffprobe.exe")

if(NOT EXISTS "${FFMPEG_EXECUTABLE}")
    message(FATAL_ERROR "[FFmpeg] ffmpeg.exe not found in ${FFMPEG_ROOT}")
endif()

if(NOT EXISTS "${FFPROBE_EXECUTABLE}")
    message(FATAL_ERROR "[FFmpeg] ffprobe.exe not found in ${FFMPEG_ROOT}")
endif()

message(STATUS "[FFmpeg] Windows x64")
message(STATUS "[FFmpeg] ffmpeg: ${FFMPEG_EXECUTABLE}")
message(STATUS "[FFmpeg] ffprobe: ${FFPROBE_EXECUTABLE}")

# Copy ffmpeg executables to output directory
add_custom_target(copy_ffmpeg ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FFMPEG_EXECUTABLE}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FFPROBE_EXECUTABLE}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/"
    COMMENT "[FFmpeg] Copying executables to output directory"
)
set_target_properties(copy_ffmpeg PROPERTIES FOLDER framework/thirdparty/ffmpeg)

install(PROGRAMS "${FFMPEG_EXECUTABLE}" DESTINATION bin)
install(PROGRAMS "${FFPROBE_EXECUTABLE}" DESTINATION bin)
