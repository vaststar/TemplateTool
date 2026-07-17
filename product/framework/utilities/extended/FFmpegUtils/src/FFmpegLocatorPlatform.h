#pragma once

#include <string>
#include <vector>

namespace ucf::utilities::ffmpeg::detail {

/// Platform-specific hooks for FFmpegLocator. Implemented once per OS in
/// FFmpegLocator_{Win,Mac,Linux}.cpp; the cross-platform search loop lives in
/// FFmpegLocator.cpp.

/// Directory of the shared library this code is linked into. Resolving relative
/// to the library (not the executable) is what lets FFmpeg be found in the
/// installed layout, where the util lives in lib/ while the app sits in bin/.
std::string libraryDirectory();

/// Executable file name for a base name, e.g. "ffmpeg" -> "ffmpeg.exe" on
/// Windows, "ffmpeg" elsewhere.
std::string executableName(const std::string& base);

/// Ordered candidate absolute paths to probe for @p exeName, given the resolved
/// library directory (which may be empty).
std::vector<std::string> candidatePaths(const std::string& exeName,
                                        const std::string& libDir);

/// Candidate paths to probe relative to an explicit application directory.
std::vector<std::string> candidatePathsForAppDir(const std::string& exeName,
                                                 const std::string& appDir);

/// True if @p path exists and is executable by this process.
bool isExecutableFile(const std::string& path);

/// Locate @p exeName by scanning the PATH environment variable.
std::string findInPath(const std::string& exeName);

} // namespace ucf::utilities::ffmpeg::detail
