# FFmpegUtils is an umbrella that groups the FFmpeg-related utilities.
#
# Two independent flavors are kept side by side so callers can pick per task:
#   - cli/  wraps the bundled ffmpeg/ffprobe executables (child process).
#   - lib/  (future) will link libav* directly for in-memory, frame-level work.
#
# They share data structures only, not behavior, so each builds its own target.
add_subdirectory(cli)

# The libav-based flavor requires the ffmpeg_lib target (dev libraries), which
# is only defined once the platform binaries are provisioned.
if(TARGET ffmpeg_lib)
    add_subdirectory(lib)
endif()
