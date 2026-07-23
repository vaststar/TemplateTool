#pragma once

/// FFmpeg Core Framework - Transcoding & Metadata Foundation
///
/// This module provides high-level transcoding support with progress tracking,
/// cancellation, and comprehensive metadata management for various media formats.
///
/// Key Components:
/// - TranscodeSession: High-level transcoding with progress tracking, multi-track support
/// - MediaMetadata:    Tag and metadata management for various formats
/// - ErrorHandler:     Centralized error management with thread-local context

#include <ucf/Utilities/FFmpegUtils/Core/ErrorHandler.h>
#include <ucf/Utilities/FFmpegUtils/Core/TranscodeSession.h>
#include <ucf/Utilities/FFmpegUtils/Core/MediaMetadata.h>

namespace ucf::utilities::ffmpeg::core {

/// Initialize FFmpeg core framework
/// Call once at application startup
void FFmpegLibUtils_EXPORT initializeCore();

/// Cleanup FFmpeg core framework
/// Call once at application shutdown
void FFmpegLibUtils_EXPORT cleanupCore();

} // namespace ucf::utilities::ffmpeg::core
