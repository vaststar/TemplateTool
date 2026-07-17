# FFmpegUtils is an umbrella that groups the FFmpeg-related utilities.
#
# Two independent flavors are kept side by side so callers can pick per task:
#   - cli/  wraps the bundled ffmpeg/ffprobe executables (child process).
#   - lib/  (future) will link libav* directly for in-memory, frame-level work.
#
# They share data structures only, not behavior, so each builds its own target.
add_subdirectory(cli)
# add_subdirectory(lib)  # enable when the libav-based flavor is added
