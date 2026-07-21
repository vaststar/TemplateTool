# =============================================================================
# FFmpeg Libs - Multi-Platform Wrapper
# =============================================================================
# Provides the FFmpeg *development libraries* (libav* shared libs + headers)
# as an INTERFACE target `ffmpeg_lib` for modules that link libav directly
# (e.g. FFmpegLibUtils). This is distinct from ../ffmpeg/, which only ships the
# command-line executables.
#
# LICENSE: LGPL + shared only. These libs are dynamically linked into our
# process; GPL / static variants would impose GPL on the application.
#
# Layout per platform/arch:
#   <platform>/<arch>/include/   (libavcodec/, libavformat/, ...)
#   <platform>/<arch>/lib/       (*.dylib / *.so* / import *.lib)
#   <platform>/<arch>/bin/       (Windows runtime *.dll)
#
# Binaries are produced by .github/workflows/build-ffmpeg-libs-all-platforms.yml
# and committed into the subdirectories below.
# =============================================================================

if(WIN32)
    add_subdirectory(windows)
elseif(APPLE)
    add_subdirectory(macosx)
elseif(UNIX)
    add_subdirectory(linux)
else()
    message(WARNING "[FFmpegLibs] Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# Public runtime aggregation target: depend on this to ensure the FFmpeg shared
# libraries are copied next to the executables before runtime.
# Usage: add_dependencies(YourTarget FFmpegLibsRuntime)
if(TARGET copy_ffmpeg_lib)
    add_custom_target(FFmpegLibsRuntime)
    add_dependencies(FFmpegLibsRuntime copy_ffmpeg_lib)
    set_target_properties(FFmpegLibsRuntime PROPERTIES FOLDER framework/thirdparty/ffmpeg)
endif()
