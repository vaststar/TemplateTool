# =============================================================================
# FFmpeg - Multi-Platform Wrapper
# =============================================================================
# Provides FFmpeg/FFprobe executables for screen capture and video processing.
# Unlike libraries, FFmpeg is a command-line tool - we only need to copy
# the binaries to the output directory.
#
# Supported platforms:
# - Windows (x64)
# - macOS (Intel, Apple Silicon via Rosetta)
# - Linux (x64)
# =============================================================================

if(WIN32)
    add_subdirectory(windows)
elseif(APPLE)
    add_subdirectory(macosx)
elseif(UNIX)
    add_subdirectory(linux)
else()
    message(WARNING "[FFmpeg] Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# Public interface target for external modules to depend on.
# Usage: add_dependencies(YourTarget FFmpegRuntime)
# This ensures ffmpeg executables are copied to the output directory
# before targets that need them at runtime.
if(TARGET copy_ffmpeg)
    add_custom_target(FFmpegRuntime)
    add_dependencies(FFmpegRuntime copy_ffmpeg)
    set_target_properties(FFmpegRuntime PROPERTIES FOLDER framework/thirdparty/ffmpeg)
endif()
