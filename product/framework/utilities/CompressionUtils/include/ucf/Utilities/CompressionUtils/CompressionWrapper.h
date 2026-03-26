#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <optional>

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

namespace ucf::utilities {

/// Compression format enumeration
enum class CompressionFormat {
    Zstd,   ///< Zstandard - high compression ratio, suitable for internal storage
    Gzip    ///< gzip (RFC 1952) - HTTP standard, suitable for external interaction
};

/// Compression level enumeration
enum class CompressionLevel : int32_t {
    Fastest = 1,      ///< Fastest speed, lowest compression ratio
    Fast    = 3,      ///< Fast compression
    Default = 5,      ///< Balanced speed and compression ratio
    Better  = 9,      ///< Better compression ratio
    Best    = 19      ///< Best compression ratio, slowest speed
};

/// Compression error codes
enum class CompressionError {
    Success = 0,
    InvalidInput,           ///< Input data is null or empty
    OutputBufferTooSmall,   ///< Output buffer capacity is insufficient
    CompressionFailed,      ///< Compression operation failed
    DecompressionFailed,    ///< Decompression operation failed
    CorruptedData,          ///< Input data is corrupted or invalid format
    UnsupportedFormat,      ///< Unsupported compression format
    UnknownError
};

/// Compression result structure
struct CompressionResult {
    std::vector<uint8_t> data;      ///< Compressed data
    size_t originalSize{0};         ///< Original uncompressed size
    size_t compressedSize{0};       ///< Compressed size
    double ratio{0.0};              ///< Compression ratio = compressedSize / originalSize
    CompressionFormat format{CompressionFormat::Zstd};  ///< Compression format used
    CompressionError error{CompressionError::Success};
    
    /// Check if compression succeeded
    bool isSuccess() const { return error == CompressionError::Success; }
    
    /// Boolean conversion for easy checking
    explicit operator bool() const { return isSuccess(); }
};

/// Decompression result structure
struct DecompressionResult {
    std::vector<uint8_t> data;      ///< Decompressed data
    size_t compressedSize{0};       ///< Original compressed size
    size_t decompressedSize{0};     ///< Decompressed size
    CompressionFormat format{CompressionFormat::Zstd};  ///< Compression format detected
    CompressionError error{CompressionError::Success};
    
    /// Check if decompression succeeded
    bool isSuccess() const { return error == CompressionError::Success; }
    
    /// Boolean conversion for easy checking
    explicit operator bool() const { return isSuccess(); }
};

/// CompressionWrapper - Compression library wrapper class
/// 
/// Provides a unified compression/decompression interface that hides
/// the underlying implementation (zstd/zlib-ng).
/// 
/// Features:
/// - PIMPL pattern to hide implementation details
/// - Multiple format support (Zstd, Gzip)
/// - No backend headers exposed in public interface
/// - Thread-safe for different instances
/// 
/// Usage example:
/// @code
/// // Zstd compression (default, for internal storage)
/// CompressionWrapper zstd;
/// auto result = zstd.compress(data.data(), data.size());
/// 
/// // Gzip compression (for HTTP interaction)
/// CompressionWrapper gzip(CompressionFormat::Gzip);
/// auto result = gzip.compress(jsonBody.data(), jsonBody.size());
/// // Set Content-Encoding: gzip header
/// @endcode
///
class Utilities_EXPORT CompressionWrapper final {
public:
    /// Create a compressor with specified format and level
    /// @param format Compression format (default: Zstd)
    /// @param level Compression level (default: Default)
    explicit CompressionWrapper(CompressionFormat format = CompressionFormat::Zstd,
                                 CompressionLevel level = CompressionLevel::Default);
    
    ~CompressionWrapper();
    
    // Non-copyable
    CompressionWrapper(const CompressionWrapper&) = delete;
    CompressionWrapper& operator=(const CompressionWrapper&) = delete;
    
    // Movable
    CompressionWrapper(CompressionWrapper&&) noexcept;
    CompressionWrapper& operator=(CompressionWrapper&&) noexcept;

    /// Set compression level
    /// @param level New compression level
    void setLevel(CompressionLevel level);
    
    /// Get current compression level
    /// @return Current compression level
    CompressionLevel getLevel() const;
    
    /// Get current compression format
    /// @return Current compression format
    CompressionFormat getFormat() const;

    // ========== Compression API ==========
    
    /// Compress data from raw pointer
    /// @param input Input data pointer
    /// @param inputSize Input data size in bytes
    /// @return CompressionResult, check isSuccess() to verify
    CompressionResult compress(const uint8_t* input, size_t inputSize) const;
    
    /// Compress data from vector
    /// @param input Input data vector
    /// @return CompressionResult, check isSuccess() to verify
    CompressionResult compress(const std::vector<uint8_t>& input) const;
    
    /// Compress string data
    /// @param input Input string
    /// @return CompressionResult, check isSuccess() to verify
    CompressionResult compress(const std::string& input) const;
    
    /// Compress to pre-allocated buffer (zero-copy)
    /// @param input Input data pointer
    /// @param inputSize Input data size
    /// @param output Output buffer pointer
    /// @param outputCapacity Output buffer capacity
    /// @param[out] compressedSize Actual compressed size
    /// @return Error code
    CompressionError compressTo(const uint8_t* input, size_t inputSize,
                                 uint8_t* output, size_t outputCapacity,
                                 size_t& compressedSize) const;

    // ========== Decompression API ==========
    
    /// Decompress data from raw pointer
    /// @param input Compressed data pointer
    /// @param inputSize Compressed data size
    /// @param originalSize Original size hint (0 = auto-detect)
    /// @return DecompressionResult, check isSuccess() to verify
    DecompressionResult decompress(const uint8_t* input, size_t inputSize,
                                    size_t originalSize = 0) const;
    
    /// Decompress data from vector
    /// @param input Compressed data vector
    /// @param originalSize Original size hint (0 = auto-detect)
    /// @return DecompressionResult, check isSuccess() to verify
    DecompressionResult decompress(const std::vector<uint8_t>& input,
                                    size_t originalSize = 0) const;
    
    /// Decompress to string
    /// @param input Compressed data pointer
    /// @param inputSize Compressed data size
    /// @param originalSize Original size hint (0 = auto-detect)
    /// @return Decompressed string, or nullopt on failure
    std::optional<std::string> decompressToString(const uint8_t* input, size_t inputSize,
                                                   size_t originalSize = 0) const;
    
    /// Decompress to pre-allocated buffer (zero-copy)
    /// @param input Compressed data pointer
    /// @param inputSize Compressed data size
    /// @param output Output buffer pointer
    /// @param outputCapacity Output buffer capacity
    /// @param[out] decompressedSize Actual decompressed size
    /// @return Error code
    CompressionError decompressTo(const uint8_t* input, size_t inputSize,
                                   uint8_t* output, size_t outputCapacity,
                                   size_t& decompressedSize) const;

    // ========== Utility Methods ==========
    
    /// Get the maximum compressed size for given input size
    /// @param inputSize Input data size
    /// @return Maximum possible compressed size
    size_t getCompressBound(size_t inputSize) const;
    
    /// Get the original size from compressed data (if available)
    /// @param compressedData Compressed data pointer
    /// @param compressedSize Compressed data size
    /// @return Original size, or 0 if unknown
    /// @note Gzip format may not be able to get original size
    size_t getDecompressedSize(const uint8_t* compressedData, size_t compressedSize) const;
    
    /// Get the compression backend name
    /// @return Backend name (e.g., "zstd", "zlib-ng")
    std::string getBackendName() const;
    
    /// Get the compression backend version
    /// @return Backend version string
    std::string getBackendVersion() const;
    
    /// Convert error code to human-readable string
    /// @param error Error code
    /// @return Error description
    const char* errorToString(CompressionError error) const;
    
    /// Convert format to human-readable string
    /// @param format Compression format
    /// @return Format name (e.g., "zstd", "gzip")
    const char* formatToString(CompressionFormat format) const;

private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
};

// ========== Convenience Functions ==========

/// Quick compress data with default settings
/// @param input Input data pointer
/// @param inputSize Input data size
/// @param format Compression format (default: Zstd)
/// @param level Compression level (default: Default)
/// @return CompressionResult
Utilities_EXPORT CompressionResult 
compressData(const uint8_t* input, size_t inputSize,
             CompressionFormat format = CompressionFormat::Zstd,
             CompressionLevel level = CompressionLevel::Default);

/// Quick decompress data
/// @param input Compressed data pointer
/// @param inputSize Compressed data size
/// @param format Compression format (must match compression format)
/// @param originalSize Original size hint (0 = auto-detect)
/// @return DecompressionResult
Utilities_EXPORT DecompressionResult 
decompressData(const uint8_t* input, size_t inputSize,
               CompressionFormat format = CompressionFormat::Zstd,
               size_t originalSize = 0);

/// Auto-detect format and decompress data
/// @param input Compressed data pointer
/// @param inputSize Compressed data size
/// @param originalSize Original size hint (0 = auto-detect)
/// @return DecompressionResult with detected format
/// @note Detects via magic bytes: zstd (0x28 0xB5), gzip (0x1F 0x8B)
Utilities_EXPORT DecompressionResult 
decompressDataAuto(const uint8_t* input, size_t inputSize,
                   size_t originalSize = 0);

/// Detect compression format from data
/// @param data Data pointer
/// @param size Data size
/// @return Detected format, or nullopt if unknown
Utilities_EXPORT std::optional<CompressionFormat>
detectCompressionFormat(const uint8_t* data, size_t size);

} // namespace ucf::utilities
