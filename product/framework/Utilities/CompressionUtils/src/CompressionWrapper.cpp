#include <ucf/Utilities/CompressionUtils/CompressionWrapper.h>
#include "CompressionUtilsLogger.h"

#include <zstd.h>
#include <zstd_errors.h>

namespace ucf::utilities {

//============================================
// Impl - PIMPL implementation
//============================================
class CompressionWrapper::Impl {
public:
    explicit Impl(CompressionLevel level)
        : mLevel(level)
        , mCCtx(ZSTD_createCCtx())
        , mDCtx(ZSTD_createDCtx())
    {
        COMPRESSION_LOG_DEBUG("CompressionWrapper created, level=" << static_cast<int>(level));
    }

    ~Impl()
    {
        if (mCCtx) ZSTD_freeCCtx(mCCtx);
        if (mDCtx) ZSTD_freeDCtx(mDCtx);
        COMPRESSION_LOG_DEBUG("CompressionWrapper destroyed");
    }

    void setLevel(CompressionLevel level) { mLevel = level; }
    CompressionLevel getLevel() const { return mLevel; }

    CompressionResult compress(const uint8_t* input, size_t inputSize) const
    {
        CompressionResult result;
        
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
            COMPRESSION_LOG_ERROR("Compression failed: " << ZSTD_getErrorName(compressedSize));
            result.error = CompressionError::CompressionFailed;
            result.data.clear();
            return result;
        }

        result.data.resize(compressedSize);
        result.originalSize = inputSize;
        result.compressedSize = compressedSize;
        result.ratio = static_cast<double>(compressedSize) / static_cast<double>(inputSize);
        result.error = CompressionError::Success;

        COMPRESSION_LOG_DEBUG("Compressed: " << inputSize << " -> " << compressedSize 
                              << " (ratio=" << result.ratio << ")");

        return result;
    }

    CompressionError compressTo(const uint8_t* input, size_t inputSize,
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
            COMPRESSION_LOG_ERROR("Compression failed: " << ZSTD_getErrorName(result));
            return CompressionError::CompressionFailed;
        }

        compressedSize = result;
        return CompressionError::Success;
    }

    DecompressionResult decompress(const uint8_t* input, size_t inputSize,
                                    size_t originalSize) const
    {
        DecompressionResult result;
        
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
            COMPRESSION_LOG_ERROR("Decompression failed: " << ZSTD_getErrorName(actualSize));
            result.error = CompressionError::DecompressionFailed;
            result.data.clear();
            return result;
        }

        result.data.resize(actualSize);
        result.compressedSize = inputSize;
        result.decompressedSize = actualSize;
        result.error = CompressionError::Success;

        COMPRESSION_LOG_DEBUG("Decompressed: " << inputSize << " -> " << actualSize);

        return result;
    }

    CompressionError decompressTo(const uint8_t* input, size_t inputSize,
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
            COMPRESSION_LOG_ERROR("Decompression failed: " << ZSTD_getErrorName(result));
            return CompressionError::DecompressionFailed;
        }

        decompressedSize = result;
        return CompressionError::Success;
    }

private:
    CompressionLevel mLevel;
    ZSTD_CCtx* mCCtx;
    ZSTD_DCtx* mDCtx;
};

//============================================
// CompressionWrapper - Public interface
//============================================
CompressionWrapper::CompressionWrapper(CompressionLevel level)
    : mImpl(std::make_unique<Impl>(level))
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

size_t CompressionWrapper::getCompressBound(size_t inputSize)
{
    return ZSTD_compressBound(inputSize);
}

size_t CompressionWrapper::getDecompressedSize(const uint8_t* compressedData, size_t compressedSize)
{
    unsigned long long frameSize = ZSTD_getFrameContentSize(compressedData, compressedSize);
    if (frameSize == ZSTD_CONTENTSIZE_ERROR || frameSize == ZSTD_CONTENTSIZE_UNKNOWN) {
        return 0;
    }
    return static_cast<size_t>(frameSize);
}

std::string CompressionWrapper::getBackendName()
{
    return "zstd";
}

std::string CompressionWrapper::getBackendVersion()
{
    return ZSTD_versionString();
}

const char* CompressionWrapper::errorToString(CompressionError error)
{
    switch (error) {
        case CompressionError::Success:              return "Success";
        case CompressionError::InvalidInput:         return "Invalid input";
        case CompressionError::OutputBufferTooSmall: return "Output buffer too small";
        case CompressionError::CompressionFailed:    return "Compression failed";
        case CompressionError::DecompressionFailed:  return "Decompression failed";
        case CompressionError::CorruptedData:        return "Corrupted data";
        case CompressionError::UnknownError:         return "Unknown error";
        default:                                     return "Invalid error code";
    }
}

//============================================
// Convenience functions
//============================================
CompressionResult compressData(const uint8_t* input, size_t inputSize, CompressionLevel level)
{
    CompressionWrapper compressor(level);
    return compressor.compress(input, inputSize);
}

DecompressionResult decompressData(const uint8_t* input, size_t inputSize, size_t originalSize)
{
    CompressionWrapper compressor;
    return compressor.decompress(input, inputSize, originalSize);
}

} // namespace ucf::utilities
