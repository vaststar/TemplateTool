#pragma once

/// FFmpeg Core Framework - Phase 1 Foundation
///
/// This module provides the fundamental architecture for building media processing
/// pipelines with proper error handling, metadata management, and transcoding support.
///
/// Key Components:
/// - MediaElement:    Abstract base for all processing units (decoders, encoders, filters)
/// - MediaPipeline:   Framework for connecting elements and orchestrating processing
/// - ErrorHandler:    Centralized error management and recovery strategies
/// - TranscodeSession: High-level transcoding with progress tracking
/// - MediaMetadata:   Tag and metadata management for various formats

#include "MediaElement.h"
#include "MediaPipeline.h"
#include "ErrorHandler.h"
#include "TranscodeSession.h"
#include "MediaMetadata.h"

namespace ucf::utilities::ffmpeg::core {

/// Initialize FFmpeg core framework
/// Call once at application startup
void FFmpegLibUtils_EXPORT initializeCore();

/// Cleanup FFmpeg core framework
/// Call once at application shutdown
void FFmpegLibUtils_EXPORT cleanupCore();

} // namespace ucf::utilities::ffmpeg::core
