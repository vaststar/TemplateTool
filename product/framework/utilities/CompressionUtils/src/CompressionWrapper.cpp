#include <ucf/Utilities/CompressionUtils/CompressionWrapper.h>
#include "CompressionUtilsLogger.h"

#include <zstd.h>
#include <zstd_errors.h>
#include <zlib.h>

namespace ucf::utilities {

//============================================
// Helper functions
//============================================
namespace {

int levelToZlibLevel(CompressionLevel level)
{
    switch (level) {
        case CompressionLevel::Fastest: return 1;
        case CompressionLevel::Fast:    return 3;
        case CompressionLevel::Default: return 6;
        case CompressionLevel::Better:  return 8;
        case CompressionLevel::Best:    return 9;
        default:                        return Z_DEFAULT_COMPRESSION;
    }
}

} // anonymous namespace

//============================================
// Impl - PIMPL implementation
//============================================
class CompressionWrapper::Impl {
public:
    explicit Impl(CompressionFormat format, CompressionLevel level)
        : mFormat(format)
        , mLevel(level)
        , mCCtx(nullptr)
        , mDCtx(nullptr)
    {
        if (format == CompressionFormat::Zstd) {
            mCCtx = ZSTD_createCCtx();
            mDCtx = ZSTD_createDCtx();
        }
        COMPRESSION_LOG_DEBUG("CompressionWrapper created, format=" 
                              << (format == CompressionFormat::Zstd ? "zstd" : "gzip")
                              << ", level=" << static_cast<int>(level));
    }

    ~Impl()
    {
        if (mCCtx) ZSTD_freeCCtx(mCCtx);
        if (mDCtx) ZSTD_freeDCtx(mDCtx);
        COMPRESSION_LOG_DEBUG("CompressionWrapper destroyed");
    }

    void setLevel(CompressionLevel level) { mLevel = level; }
    CompressionLevel getLevel() const { return mLevel; }
    CompressionFormat getFormat() const { return mFormat; }

    CompressionResult compress(const uint8_t* input, size_t inputSize) const
    {
        if (mFormat == CompressionFormat::Zstd) {
            return compressZstd(input, inputSize);
        } else {
            return compressGzip(input, inputSize);
        }
    }

    CompressionError compressTo(const uint8_t* input, size_t inputSize,
                                 uint8_t* output, size_t outputCapacity,
                                 size_t& compressedSize) const
    {
        if (mFormat == CompressionFormat::Zstd) {
            return compressToZstd(input, inputSize, output, outputCapacity, compressedSize);
        } else {
            return compressToGzip(input, inputSize, output, outputCapacity, compressedSize);
        }
    }

    DecompressionResult decompress(const uint8_t* input, size_t inputSize,
                                    size_t originalSize) const
    {
        if (mFormat == CompressionFormat::Zstd) {
            return decompressZstd(input, inputSize, originalSize);
        } else {
            return decompressGzip(input, inputSize, originalSize);
        }
    }

    CompressionError decompressTo(const uint8_t* input, size_t inputSize,
                                   uint8_t* output, size_t outputCapacity,
                                   size_t& decompressedSize) const
    {
        if (mFormat == CompressionFormat::Zstd) {
            return decompressToZstd(input, inputSize, output, outputCapacity, decompressedSize);
        } else {
            return decompressToGzip(input, inputSize, output, outputCapacity, decompressedSize);
        }
    }

    size_t getCompressBound(size_t inputSize) const
    {
        if (mFormat == CompressionFormat::Zstd) {
            return ZSTD_compressBound(inputSize);
        } else {
            // gzip: deflate bound + 18 bytes for gzip header/trailer
            return compressBound(static_cast<uLong>(inputSize)) + 18;
        }
    }

    size_t getDecompressedSize(const uint8_t* compressedData, size_t compressedSize) const
    {
        if (mFormat == CompressionFormat::Zstd) {
            unsigned long long frameSize = ZSTD_getFrameContentSize(compressedData, compressedSize);
            if (frameSize == ZSTD_CONTENTSIZE_ERROR || frameSize == ZSTD_CONTENTSIZE_UNKNOWN) {
                return 0;
            }
            return static_cast<size_t>(frameSize);
        } else {
            // Gzip stores uncompressed size in last 4 bytes (mod 2^32)
            // This is unreliable for files > 4GB
            if (compressedSize < 4) return 0;
            const uint8_t* footer = compressedData + compressedSize - 4;
            return static_cast<size_t>(footer[0]) |
                   (static_cast<size_t>(footer[1]) << 8) |
                   (static_cast<size_t>(footer[2]) << 16) |
                   (static_cast<size_t>(footer[3]) << 24);
        }
    }

private:
    //========== Zstd Implementation ==========
    
    CompressionResult compressZstd(const uint8_t* input, size_t inputSize) const
    {
        CompressionResult result;
        result.format = CompressionFormat::Zstd;
        
        if (input == nullptr || inputSize == 0) {
            result.error = CompressionError::InvalidInput;
            return result;
        }

        size_t bound = ZSTD_compressBound(inputSize);
        result.data.resize(bound);

        size_t compressedSize = ZSTD_compressCCtx(
            mCCtx,
            result.data.data(), result.data.size(),
            input, inputSize,
            static_cast<int>(mLevel)
        );

        if (ZSTD_isError(compressedSize)) {
            COMPRESSION_LOG_ERROR("Zstd compression failed: " << ZSTD_getErrorName(compressedSize));
            result.error = CompressionError::CompressionFailed;
            result.data.clear();
            return result;
        }

        result.data.resize(compressedSize);
        result.originalSize = inputSize;
        result.compressedSize = compressedSize;
        result.ratio = static_cast<double>(compressedSize) / static_cast<double>(inputSize);
        result.error = CompressionError::Success;

        COMPRESSION_LOG_DEBUG("Zstd compressed: " << inputSize << " -> " << compressedSize 
                              << " (ratio=" << result.ratio << ")");

        return result;
    }

    CompressionError compressToZstd(const uint8_t* input, size_t inputSize,
                                     uint8_t* output, size_t outputCapacity,
                                     size_t& compressedSize) const
    {
        if (input == nullptr || inputSize == 0) {
            return CompressionError::InvalidInput;
        }

        size_t result = ZSTD_compressCCtx(
            mCCtx,
            output, outputCapacity,
            input, inputSize,
            static_cast<int>(mLevel)
        );

        if (ZSTD_isError(result)) {
            if (ZSTD_getErrorCode(result) == ZSTD_error_dstSize_tooSmall) {
                return CompressionError::OutputBufferTooSmall;
            }
            COMPRESSION_LOG_ERROR("Zstd compression failed: " << ZSTD_getErrorName(result));
            return CompressionError::CompressionFailed;
        }

        compressedSize = result;
        return CompressionError::Success;
    }

    DecompressionResult decompressZstd(const uint8_t* input, size_t inputSize,
                                        size_t originalSize) const
    {
        DecompressionResult result;
        result.format = CompressionFormat::Zstd;
        
        if (input == nullptr || inputSize == 0) {
            result.error = CompressionError::InvalidInput;
            return result;
        }

        // Try to get original size from frame header
        size_t decompressedSize = originalSize;
        if (decompressedSize == 0) {
            unsigned long long frameSize = ZSTD_getFrameContentSize(input, inputSize);
            if (frameSize == ZSTD_CONTENTSIZE_ERROR) {
                COMPRESSION_LOG_ERROR("Corrupted data: cannot get frame content size");
                result.error = CompressionError::CorruptedData;
                return result;
            }
            if (frameSize == ZSTD_CONTENTSIZE_UNKNOWN) {
                // Use estimated size (assume 10x compression ratio)
                decompressedSize = inputSize * 10;
            } else {
                decompressedSize = static_cast<size_t>(frameSize);
            }
        }

        result.data.resize(decompressedSize);

        size_t actualSize = ZSTD_decompressDCtx(
            mDCtx,
            result.data.data(), result.data.size(),
            input, inputSize
        );

        if (ZSTD_isError(actualSize)) {
            COMPRESSION_LOG_ERROR("Zstd decompression failed: " << ZSTD_getErrorName(actualSize));
            result.error = CompressionError::DecompressionFailed;
            result.data.clear();
            return result;
        }

        result.data.resize(actualSize);
        result.compressedSize = inputSize;
        result.decompressedSize = actualSize;
        result.error = CompressionError::Success;

        COMPRESSION_LOG_DEBUG("Zstd decompressed: " << inputSize << " -> " << actualSize);

        return result;
    }

    CompressionError decompressToZstd(const uint8_t* input, size_t inputSize,
                                       uint8_t* output, size_t outputCapacity,
                                       size_t& decompressedSize) const
    {
        if (input == nullptr || inputSize == 0) {
            return CompressionError::InvalidInput;
        }

        size_t result = ZSTD_decompressDCtx(
            mDCtx,
            output, outputCapacity,
            input, inputSize
        );

        if (ZSTD_isError(result)) {
            if (ZSTD_getErrorCode(result) == ZSTD_error_dstSize_tooSmall) {
                return CompressionError::OutputBufferTooSmall;
            }
            COMPRESSION_LOG_ERROR("Zstd decompression failed: " << ZSTD_getErrorName(result));
            return CompressionError::DecompressionFailed;
        }

        decompressedSize = result;
        return CompressionError::Success;
    }

    //========== Gzip Implementation ==========
    
    CompressionResult compressGzip(const uint8_t* input, size_t inputSize) const
    {
        CompressionResult result;
        result.format = CompressionFormat::Gzip;
        
        if (input == nullptr || inputSize == 0) {
            result.error = CompressionError::InvalidInput;
            return result;
        }

        // Estimate output size
        uLong bound = compressBound(static_cast<uLong>(inputSize)) + 18; // +18 for gzip header/trailer
        result.data.resize(bound);

        z_stream strm{};
        strm.next_in = const_cast<Bytef*>(input);
        strm.avail_in = static_cast<uInt>(inputSize);
        strm.next_out = result.data.data();
        strm.avail_out = static_cast<uInt>(result.data.size());

        // windowBits = 15 + 16 = 31 for gzip format
        int ret = deflateInit2(&strm, levelToZlibLevel(mLevel), Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
        if (ret != Z_OK) {
            COMPRESSION_LOG_ERROR("Gzip deflateInit2 failed: " << ret);
            result.error = CompressionError::CompressionFailed;
            return result;
        }

        ret = deflate(&strm, Z_FINISH);
        if (ret != Z_STREAM_END) {
            deflateEnd(&strm);
            COMPRESSION_LOG_ERROR("Gzip deflate failed: " << ret);
            result.error = CompressionError::CompressionFailed;
            result.data.clear();
            return result;
        }

        size_t compressedSize = strm.total_out;
        deflateEnd(&strm);

        result.data.resize(compressedSize);
        result.originalSize = inputSize;
        result.compressedSize = compressedSize;
        result.ratio = static_cast<double>(compressedSize) / static_cast<double>(inputSize);
        result.error = CompressionError::Success;

        COMPRESSION_LOG_DEBUG("Gzip compressed: " << inputSize << " -> " << compressedSize 
                              << " (ratio=" << result.ratio << ")");

        return result;
    }

    CompressionError compressToGzip(const uint8_t* input, size_t inputSize,
                                     uint8_t* output, size_t outputCapacity,
                                     size_t& compressedSize) const
    {
        if (input == nullptr || inputSize == 0) {
            return CompressionError::InvalidInput;
        }

        z_stream strm{};
        strm.next_in = const_cast<Bytef*>(input);
        strm.avail_in = static_cast<uInt>(inputSize);
        strm.next_out = output;
        strm.avail_out = static_cast<uInt>(outputCapacity);

        int ret = deflateInit2(&strm, levelToZlibLevel(mLevel), Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
        if (ret != Z_OK) {
            return CompressionError::CompressionFailed;
        }

        ret = deflate(&strm, Z_FINISH);
        if (ret != Z_STREAM_END) {
            deflateEnd(&strm);
            if (ret == Z_BUF_ERROR) {
                return CompressionError::OutputBufferTooSmall;
            }
            return CompressionError::CompressionFailed;
        }

        compressedSize = strm.total_out;
        deflateEnd(&strm);
        return CompressionError::Success;
    }

    DecompressionResult decompressGzip(const uint8_t* input, size_t inputSize,
                                        size_t originalSize) const
    {
        DecompressionResult result;
        result.format = CompressionFormat::Gzip;
        
        if (input == nullptr || inputSize == 0) {
            result.error = CompressionError::InvalidInput;
            return result;
        }

        // Estimate decompressed size
        size_t decompressedSize = originalSize;
        if (decompressedSize == 0) {
            // Try to get from gzip footer (last 4 bytes)
            if (inputSize >= 4) {
                const uint8_t* footer = input + inputSize - 4;
                decompressedSize = static_cast<size_t>(footer[0]) |
                                   (static_cast<size_t>(footer[1]) << 8) |
                                   (static_cast<size_t>(footer[2]) << 16) |
                                   (static_cast<size_t>(footer[3]) << 24);
            }
            if (decompressedSize == 0) {
                // Fallback: assume 10x compression ratio
                decompressedSize = inputSize * 10;
            }
        }

        result.data.resize(decompressedSize);

        z_stream strm{};
        strm.next_in = const_cast<Bytef*>(input);
        strm.avail_in = static_cast<uInt>(inputSize);
        strm.next_out = result.data.data();
        strm.avail_out = static_cast<uInt>(result.data.size());

        // windowBits = 15 + 16 = 31 for gzip auto-detect
        int ret = inflateInit2(&strm, 15 + 16);
        if (ret != Z_OK) {
            COMPRESSION_LOG_ERROR("Gzip inflateInit2 failed: " << ret);
            result.error = CompressionError::DecompressionFailed;
            return result;
        }

        ret = inflate(&strm, Z_FINISH);
        
        // Handle case where output buffer was too small
        while (ret == Z_BUF_ERROR || (ret == Z_OK && strm.avail_out == 0)) {
            size_t currentSize = result.data.size();
            result.data.resize(currentSize * 2);
            strm.next_out = result.data.data() + currentSize;
            strm.avail_out = static_cast<uInt>(currentSize);
            ret = inflate(&strm, Z_FINISH);
        }

        if (ret != Z_STREAM_END) {
            inflateEnd(&strm);
            COMPRESSION_LOG_ERROR("Gzip inflate failed: " << ret);
            result.error = (ret == Z_DATA_ERROR) ? CompressionError::CorruptedData 
                                                  : CompressionError::DecompressionFailed;
            result.data.clear();
            return result;
        }

        size_t actualSize = strm.total_out;
        inflateEnd(&strm);

        result.data.resize(actualSize);
        result.compressedSize = inputSize;
        result.decompressedSize = actualSize;
        result.error = CompressionError::Success;

        COMPRESSION_LOG_DEBUG("Gzip decompressed: " << inputSize << " -> " << actualSize);

        return result;
    }

    CompressionError decompressToGzip(const uint8_t* input, size_t inputSize,
                                       uint8_t* output, size_t outputCapacity,
                                       size_t& decompressedSize) const
    {
        if (input == nullptr || inputSize == 0) {
            return CompressionError::InvalidInput;
        }

        z_stream strm{};
        strm.next_in = const_cast<Bytef*>(input);
        strm.avail_in = static_cast<uInt>(inputSize);
        strm.next_out = output;
        strm.avail_out = static_cast<uInt>(outputCapacity);

        int ret = inflateInit2(&strm, 15 + 16);
        if (ret != Z_OK) {
            return CompressionError::DecompressionFailed;
        }

        ret = inflate(&strm, Z_FINISH);
        if (ret != Z_STREAM_END) {
            inflateEnd(&strm);
            if (ret == Z_BUF_ERROR) {
                return CompressionError::OutputBufferTooSmall;
            }
            if (ret == Z_DATA_ERROR) {
                return CompressionError::CorruptedData;
            }
            return CompressionError::DecompressionFailed;
        }

        decompressedSize = strm.total_out;
        inflateEnd(&strm);
        return CompressionError::Success;
    }

private:
    CompressionFormat mFormat;
    CompressionLevel mLevel;
    ZSTD_CCtx* mCCtx;
    ZSTD_DCtx* mDCtx;
};

//============================================
// CompressionWrapper - Public interface
//============================================
CompressionWrapper::CompressionWrapper(CompressionFormat format, CompressionLevel level)
    : mImpl(std::make_unique<Impl>(format, level))
{
}

CompressionWrapper::~CompressionWrapper() = default;

CompressionWrapper::CompressionWrapper(CompressionWrapper&&) noexcept = default;
CompressionWrapper& CompressionWrapper::operator=(CompressionWrapper&&) noexcept = default;

void CompressionWrapper::setLevel(CompressionLevel level)
{
    mImpl->setLevel(level);
}

CompressionLevel CompressionWrapper::getLevel() const
{
    return mImpl->getLevel();
}

CompressionFormat CompressionWrapper::getFormat() const
{
    return mImpl->getFormat();
}

CompressionResult CompressionWrapper::compress(const uint8_t* input, size_t inputSize) const
{
    return mImpl->compress(input, inputSize);
}

CompressionResult CompressionWrapper::compress(const std::vector<uint8_t>& input) const
{
    return mImpl->compress(input.data(), input.size());
}

CompressionResult CompressionWrapper::compress(const std::string& input) const
{
    return mImpl->compress(reinterpret_cast<const uint8_t*>(input.data()), input.size());
}

CompressionError CompressionWrapper::compressTo(const uint8_t* input, size_t inputSize,
                                                 uint8_t* output, size_t outputCapacity,
                                                 size_t& compressedSize) const
{
    return mImpl->compressTo(input, inputSize, output, outputCapacity, compressedSize);
}

DecompressionResult CompressionWrapper::decompress(const uint8_t* input, size_t inputSize,
                                                    size_t originalSize) const
{
    return mImpl->decompress(input, inputSize, originalSize);
}

DecompressionResult CompressionWrapper::decompress(const std::vector<uint8_t>& input,
                                                    size_t originalSize) const
{
    return mImpl->decompress(input.data(), input.size(), originalSize);
}

std::optional<std::string> CompressionWrapper::decompressToString(const uint8_t* input, size_t inputSize,
                                                                   size_t originalSize) const
{
    auto result = mImpl->decompress(input, inputSize, originalSize);
    if (!result.isSuccess()) {
        return std::nullopt;
    }
    return std::string(result.data.begin(), result.data.end());
}

CompressionError CompressionWrapper::decompressTo(const uint8_t* input, size_t inputSize,
                                                   uint8_t* output, size_t outputCapacity,
                                                   size_t& decompressedSize) const
{
    return mImpl->decompressTo(input, inputSize, output, outputCapacity, decompressedSize);
}

size_t CompressionWrapper::getCompressBound(size_t inputSize) const
{
    return mImpl->getCompressBound(inputSize);
}

size_t CompressionWrapper::getDecompressedSize(const uint8_t* compressedData, size_t compressedSize) const
{
    return mImpl->getDecompressedSize(compressedData, compressedSize);
}

std::string CompressionWrapper::getBackendName() const
{
    if (mImpl->getFormat() == CompressionFormat::Zstd) {
        return "zstd";
    } else {
        return "zlib-ng";
    }
}

std::string CompressionWrapper::getBackendVersion() const
{
    if (mImpl->getFormat() == CompressionFormat::Zstd) {
        return ZSTD_versionString();
    } else {
        return zlibVersion();
    }
}

const char* CompressionWrapper::errorToString(CompressionError error) const
{
    switch (error) {
        case CompressionError::Success:              return "Success";
        case CompressionError::InvalidInput:         return "Invalid input";
        case CompressionError::OutputBufferTooSmall: return "Output buffer too small";
        case CompressionError::CompressionFailed:    return "Compression failed";
        case CompressionError::DecompressionFailed:  return "Decompression failed";
        case CompressionError::CorruptedData:        return "Corrupted data";
        case CompressionError::UnsupportedFormat:    return "Unsupported format";
        case CompressionError::UnknownError:         return "Unknown error";
        default:                                     return "Invalid error code";
    }
}

const char* CompressionWrapper::formatToString(CompressionFormat format) const
{
    switch (format) {
        case CompressionFormat::Zstd: return "zstd";
        case CompressionFormat::Gzip: return "gzip";
        default:                      return "unknown";
    }
}

//============================================
// Convenience functions
//============================================
CompressionResult compressData(const uint8_t* input, size_t inputSize,
                               CompressionFormat format, CompressionLevel level)
{
    CompressionWrapper compressor(format, level);
    return compressor.compress(input, inputSize);
}

DecompressionResult decompressData(const uint8_t* input, size_t inputSize,
                                   CompressionFormat format, size_t originalSize)
{
    CompressionWrapper compressor(format);
    return compressor.decompress(input, inputSize, originalSize);
}

DecompressionResult decompressDataAuto(const uint8_t* input, size_t inputSize,
                                       size_t originalSize)
{
    auto format = detectCompressionFormat(input, inputSize);
    if (!format.has_value()) {
        DecompressionResult result;
        result.error = CompressionError::UnsupportedFormat;
        return result;
    }
    
    CompressionWrapper compressor(format.value());
    return compressor.decompress(input, inputSize, originalSize);
}

std::optional<CompressionFormat> detectCompressionFormat(const uint8_t* data, size_t size)
{
    if (size < 4) {
        return std::nullopt;
    }
    
    // Check gzip magic (0x1F 0x8B)
    if (data[0] == 0x1F && data[1] == 0x8B) {
        return CompressionFormat::Gzip;
    }
    
    // Check zstd magic (0x28 0xB5 0x2F 0xFD)
    if (data[0] == 0x28 && data[1] == 0xB5 && data[2] == 0x2F && data[3] == 0xFD) {
        return CompressionFormat::Zstd;
    }
    
    return std::nullopt;
}

} // namespace ucf::utilities
