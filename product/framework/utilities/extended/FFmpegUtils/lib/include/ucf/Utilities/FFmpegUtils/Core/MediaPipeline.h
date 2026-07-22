#pragma once

#include <vector>
#include <memory>
#include <string>

#include <ucf/Utilities/FFmpegUtils/Lib/FFmpegExport.h>
#include "MediaElement.h"

namespace ucf::utilities::ffmpeg::core {

/// Pipeline for connecting and orchestrating media processing elements (decoders, filters, encoders, etc).
/// Supports linear pipelines and branching topologies.
class FFmpegLibUtils_EXPORT MediaPipeline
{
public:
    MediaPipeline();
    ~MediaPipeline();

    MediaPipeline(const MediaPipeline&) = delete;
    MediaPipeline& operator=(const MediaPipeline&) = delete;

    // ========== Building ==========

    /// Add element to pipeline. Returns element index for use in connect().
    /// Elements are processed in order of addition.
    int addElement(MediaElementPtr element);

    /// Connect output of one element to input of another.
    /// For simple pipeline: connect(0, 1), connect(1, 2), etc.
    /// @param srcIdx    Index of source element
    /// @param destIdx   Index of destination element
    /// @return true if connection successful
    bool connect(int srcIdx, int destIdx);

    /// Get total number of elements
    int getElementCount() const;

    /// Get element by index
    MediaElementPtr getElement(int index) const;

    // ========== Processing ==========

    /// Open first element (typically Demuxer) with source
    bool open(const std::string& source);

    /// Process entire pipeline until EOF or error.
    /// Processes all connected elements in sequence.
    /// @return true if completed successfully, false on error
    bool process();

    /// Process single frame/chunk through pipeline
    /// @return true if processed, false if EOF or error
    bool processOne();

    /// Close all elements
    void close();

    // ========== State ==========

    /// Check if pipeline is open
    bool isOpen() const;

    /// Check if entire pipeline reached EOF
    bool isEof() const;

    /// Get progress (0.0 to 1.0) - requires MediaElement implementations to support it
    float getProgress() const;

    // ========== Error Handling ==========

    /// Get last error from any element in pipeline
    std::string lastError() const;

    /// Get element that caused last error (-1 if none)
    int getErrorElementIndex() const;

    // ========== Debugging ==========

    /// Get pipeline description (ASCII diagram)
    std::string getDescription() const;

private:
    struct PipelineElement {
        MediaElementPtr element;
        std::vector<int> connectedTo;  // Output indices
    };

    std::vector<PipelineElement> mElements;
    int mErrorElementIdx = -1;
    std::string mLastError;
    bool mIsOpen = false;
};

using MediaPipelinePtr = std::shared_ptr<MediaPipeline>;

} // namespace ucf::utilities::ffmpeg::core
