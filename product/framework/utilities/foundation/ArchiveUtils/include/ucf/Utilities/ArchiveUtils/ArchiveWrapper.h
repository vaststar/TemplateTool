#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

namespace ucf::utilities {

/// Archive error codes
enum class ArchiveError {
    Success = 0,
    InvalidPath,            ///< Invalid file path
    FileNotFound,           ///< Source file not found
    FileOpenFailed,         ///< Failed to open file
    FileReadFailed,         ///< Failed to read file
    FileWriteFailed,        ///< Failed to write file
    ArchiveCreateFailed,    ///< Failed to create archive
    ArchiveOpenFailed,      ///< Failed to open archive
    ArchiveCorrupted,       ///< Archive file is corrupted
    EntryNotFound,          ///< Entry not found in archive
    ExtractionFailed,       ///< Failed to extract entry
    CompressionFailed,      ///< Failed to compress entry
    UnsupportedFormat,      ///< Unsupported archive format
    PermissionDenied,       ///< Permission denied
    OutOfMemory,            ///< Out of memory
    UnknownError
};

/// Archive entry information
struct ArchiveEntry {
    std::string name;           ///< Entry name (relative path in archive)
    uint64_t compressedSize{0}; ///< Compressed size in bytes
    uint64_t uncompressedSize{0}; ///< Uncompressed size in bytes
    uint32_t crc32{0};          ///< CRC32 checksum
    bool isDirectory{false};    ///< Whether this entry is a directory
    int64_t modifiedTime{0};    ///< Last modified time (Unix timestamp)
};

/// Archive creation options
struct ArchiveOptions {
    int compressionLevel{6};    ///< Compression level (1-9, default: 6)
    bool storeSymlinks{false};  ///< Store symlinks as links (not target content)
    bool preserveTime{true};    ///< Preserve file modification time
    std::string password;       ///< Password for encryption (empty = no encryption)
    std::string comment;        ///< Archive comment
};

/// Progress callback for archive operations
/// @param current Current entry index (0-based)
/// @param total Total number of entries
/// @param entryName Current entry name
/// @return false to cancel operation, true to continue
using ArchiveProgressCallback = std::function<bool(size_t current, size_t total, const std::string& entryName)>;

/// ArchiveWrapper - ZIP archive wrapper class
/// 
/// Provides a unified archive interface that hides the underlying
/// implementation (minizip-ng).
/// 
/// Features:
/// - PIMPL pattern to hide implementation details
/// - No minizip headers exposed in public interface
/// - Thread-safe for different instances
/// 
/// Usage example:
/// @code
/// ArchiveWrapper archiver;
/// 
/// // Create a ZIP file
/// std::vector<std::string> files = {"file1.txt", "file2.txt"};
/// auto error = archiver.create("output.zip", files);
/// 
/// // Extract a ZIP file
/// error = archiver.extractAll("archive.zip", "output_dir");
/// 
/// // List contents
/// auto entries = archiver.list("archive.zip");
/// @endcode
///
class Utilities_EXPORT ArchiveWrapper final {
public:
    ArchiveWrapper();
    ~ArchiveWrapper();

    // Non-copyable
    ArchiveWrapper(const ArchiveWrapper&) = delete;
    ArchiveWrapper& operator=(const ArchiveWrapper&) = delete;

    // Movable
    ArchiveWrapper(ArchiveWrapper&&) noexcept;
    ArchiveWrapper& operator=(ArchiveWrapper&&) noexcept;

    /// Set archive options
    /// @param options Archive options
    void setOptions(const ArchiveOptions& options);

    /// Get current archive options
    /// @return Current options
    ArchiveOptions getOptions() const;

    /// Set progress callback
    /// @param callback Progress callback function
    void setProgressCallback(ArchiveProgressCallback callback);

    // ========== Create Archive API ==========

    /// Create archive from a list of files
    /// @param archivePath Output archive path
    /// @param filePaths List of file/directory paths to add
    /// @param basePath Base path to strip from entry names (optional)
    /// @return Error code
    ArchiveError create(const std::string& archivePath,
                        const std::vector<std::string>& filePaths,
                        const std::string& basePath = "");

    /// Create archive from a single directory
    /// @param archivePath Output archive path
    /// @param directoryPath Directory to archive
    /// @param includeRootDir Include root directory name in archive
    /// @return Error code
    ArchiveError createFromDirectory(const std::string& archivePath,
                                      const std::string& directoryPath,
                                      bool includeRootDir = false);

    /// Add data from memory to archive
    /// @param archivePath Archive path (will create if not exists)
    /// @param entryName Entry name in archive
    /// @param data Data to add
    /// @param dataSize Data size in bytes
    /// @return Error code
    ArchiveError addFromMemory(const std::string& archivePath,
                                const std::string& entryName,
                                const uint8_t* data,
                                size_t dataSize);

    // ========== Extract Archive API ==========

    /// Extract all entries from archive
    /// @param archivePath Archive path
    /// @param destDir Destination directory
    /// @return Error code
    ArchiveError extractAll(const std::string& archivePath,
                            const std::string& destDir);

    /// Extract a single entry from archive
    /// @param archivePath Archive path
    /// @param entryName Entry name to extract
    /// @param destPath Destination file path
    /// @return Error code
    ArchiveError extractEntry(const std::string& archivePath,
                               const std::string& entryName,
                               const std::string& destPath);

    /// Extract entry to memory
    /// @param archivePath Archive path
    /// @param entryName Entry name to extract
    /// @param[out] data Output data vector
    /// @return Error code
    ArchiveError extractToMemory(const std::string& archivePath,
                                  const std::string& entryName,
                                  std::vector<uint8_t>& data);

    // ========== Query API ==========

    /// List all entries in archive
    /// @param archivePath Archive path
    /// @return List of archive entries (empty on error)
    std::vector<ArchiveEntry> list(const std::string& archivePath);

    /// Check if entry exists in archive
    /// @param archivePath Archive path
    /// @param entryName Entry name to check
    /// @return true if entry exists
    bool hasEntry(const std::string& archivePath,
                  const std::string& entryName);

    /// Get entry information
    /// @param archivePath Archive path
    /// @param entryName Entry name
    /// @param[out] entry Entry information
    /// @return Error code
    ArchiveError getEntryInfo(const std::string& archivePath,
                               const std::string& entryName,
                               ArchiveEntry& entry);

    // ========== Utility Methods ==========

    /// Check if file is a valid ZIP archive
    /// @param filePath File path to check
    /// @return true if valid ZIP archive
    static bool isValidArchive(const std::string& filePath);

    /// Get the archive backend name
    /// @return Backend name (e.g., "minizip-ng")
    static std::string getBackendName();

    /// Get the archive backend version
    /// @return Backend version string
    static std::string getBackendVersion();

    /// Convert error code to human-readable string
    /// @param error Error code
    /// @return Error description
    static const char* errorToString(ArchiveError error);

private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
};

// ========== Convenience Functions ==========

/// Quick create ZIP archive
/// @param archivePath Output archive path
/// @param filePaths Files to add
/// @return Error code
Utilities_EXPORT ArchiveError 
createZipArchive(const std::string& archivePath,
                 const std::vector<std::string>& filePaths);

/// Quick extract ZIP archive
/// @param archivePath Archive path
/// @param destDir Destination directory
/// @return Error code
Utilities_EXPORT ArchiveError 
extractZipArchive(const std::string& archivePath,
                  const std::string& destDir);

} // namespace ucf::utilities
