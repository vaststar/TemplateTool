# FFmpeg - all platforms
#
# Two independent deliverables live side by side:
#   - ffmpeg/       command-line executables (ffmpeg/ffprobe), copied at build.
#   - ffmpeg-libs/  development libraries (libav* shared libs + headers) that
#                   modules link against. LGPL shared builds only.
add_subdirectory(ffmpeg)

# ffmpeg-libs is optional: the target `ffmpeg_lib` is only defined once the
# platform binaries have been committed (via build-ffmpeg-libs-* workflow).
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg-libs/CMakeLists.txt")
    add_subdirectory(ffmpeg-libs)
endif()
