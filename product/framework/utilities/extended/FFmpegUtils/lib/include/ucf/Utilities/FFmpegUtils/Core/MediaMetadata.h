#pragma once

#include <string>
#include <map>
#include <optional>
#include <vector>

#include <ucf/Utilities/FFmpegUtils/Lib/FFmpegExport.h>
#include <ucf/Utilities/FFmpegUtils/Lib/FFmpegTypes.h>

namespace ucf::utilities::ffmpeg::core {

/// Media file metadata (ID3, MP4 atoms, Matroska tags, etc).
/// Supports reading and writing common tags across multiple formats.
class FFmpegLibUtils_EXPORT MediaMetadata
{
public:
    MediaMetadata() = default;
    ~MediaMetadata() = default;

    // ========== Standard Tags ==========

    /// Common metadata fields
    std::string title;
    std::string artist;
    std::string album;
    std::string albumArtist;
    std::string genre;
    std::string date;              // YYYY or YYYY-MM-DD
    int year = 0;                  // Extracted from date
    int trackNumber = 0;
    int discNumber = 0;
    std::string comment;
    std::string composer;
    std::string copyright;
    std::string duration;          // HH:MM:SS format

    // ========== Media Info ==========

    struct AttachedImage
    {
        std::string mimeType;      // e.g. "image/jpeg"
        std::vector<uint8_t> data; // Binary image data
        std::string description;   // e.g. "front cover"
    };

    std::vector<AttachedImage> attachedImages;

    // ========== Custom Tags ==========

    /// Get custom tag value (for non-standard tags)
    std::optional<std::string> getTag(const std::string& key) const;

    /// Set custom tag value
    void setTag(const std::string& key, const std::string& value);

    /// Get all custom tags
    const std::map<std::string, std::string>& getAllTags() const;

    /// Clear all custom tags
    void clearCustomTags();

    // ========== I/O ==========

    /// Read metadata from file (supports MP3, M4A, FLAC, OGG, Matroska, etc)
    /// @return true if read successfully, false on error (check getError())
    bool readFromFile(const std::string& filePath);

    /// Write metadata back to file (may create backup depending on format)
    /// @return true if written successfully
    bool writeToFile(const std::string& filePath);

    /// Extract metadata from media file without modifying it
    static std::optional<MediaMetadata> fromFile(const std::string& filePath);

    // ========== Errors ==========

    /// Get last error message
    std::string getError() const;

    // ========== Utility ==========

    /// Get all metadata as formatted string (for display/debugging)
    std::string toString() const;

    /// Check if any metadata is set
    bool isEmpty() const;

    /// Clear all metadata
    void clear();

private:
    std::map<std::string, std::string> mCustomTags;
    std::string mLastError;
};

using MediaMetadataPtr = std::shared_ptr<MediaMetadata>;

} // namespace ucf::utilities::ffmpeg::core
