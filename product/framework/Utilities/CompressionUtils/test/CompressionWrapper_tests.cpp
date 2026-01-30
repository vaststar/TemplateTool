#include <catch2/catch_test_macros.hpp>
#include <ucf/Utilities/CompressionUtils/CompressionWrapper.h>

#include <random>
#include <string>
#include <vector>
#include <numeric>

using namespace ucf::utilities;

//============================================
// Helper Functions
//============================================
namespace {

std::vector<uint8_t> generateRandomData(size_t size) {
    std::vector<uint8_t> data(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    for (auto& byte : data) {
        byte = static_cast<uint8_t>(dis(gen));
    }
    return data;
}

std::vector<uint8_t> generateCompressibleData(size_t size) {
    // Highly compressible: repeating pattern
    std::vector<uint8_t> data(size);
    for (size_t i = 0; i < size; ++i) {
        data[i] = static_cast<uint8_t>(i % 10);
    }
    return data;
}

std::string generateRandomString(size_t size) {
    static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string result(size, ' ');
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(charset) - 2);
    for (auto& c : result) {
        c = charset[dis(gen)];
    }
    return result;
}

} // anonymous namespace

//============================================
// Compression Tests
//============================================
TEST_CASE("CompressionWrapper - Compress raw pointer", "[CompressionUtils][Compress]") {
    CompressionWrapper compressor;
    
    std::string data = "Hello World! This is a test string for compression.";
    auto result = compressor.compress(
        reinterpret_cast<const uint8_t*>(data.data()), 
        data.size()
    );
    
    REQUIRE(result.isSuccess());
    REQUIRE(result.error == CompressionError::Success);
    REQUIRE(result.originalSize == data.size());
    REQUIRE(result.compressedSize > 0);
    REQUIRE(result.compressedSize == result.data.size());
    REQUIRE(result.ratio > 0.0);
}

TEST_CASE("CompressionWrapper - Compress vector", "[CompressionUtils][Compress]") {
    CompressionWrapper compressor;
    
    std::vector<uint8_t> data = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'};
    auto result = compressor.compress(data);
    
    REQUIRE(result.isSuccess());
    REQUIRE(result.originalSize == data.size());
}

TEST_CASE("CompressionWrapper - Compress string", "[CompressionUtils][Compress]") {
    CompressionWrapper compressor;
    
    std::string data = "This is a string to compress";
    auto result = compressor.compress(data);
    
    REQUIRE(result.isSuccess());
    REQUIRE(result.originalSize == data.size());
}

TEST_CASE("CompressionWrapper - Compress to pre-allocated buffer", "[CompressionUtils][Compress]") {
    CompressionWrapper compressor;
    
    std::string data = "Test data for compression";
    size_t bound = compressor.getCompressBound(data.size());
    
    std::vector<uint8_t> output(bound);
    size_t compressedSize = 0;
    
    auto error = compressor.compressTo(
        reinterpret_cast<const uint8_t*>(data.data()),
        data.size(),
        output.data(),
        output.size(),
        compressedSize
    );
    
    REQUIRE(error == CompressionError::Success);
    REQUIRE(compressedSize > 0);
    REQUIRE(compressedSize <= bound);
}

TEST_CASE("CompressionWrapper - Compress highly compressible data", "[CompressionUtils][Compress]") {
    CompressionWrapper compressor;
    
    auto data = generateCompressibleData(10000);
    auto result = compressor.compress(data);
    
    REQUIRE(result.isSuccess());
    REQUIRE(result.ratio < 1.0); // Should compress well
    REQUIRE(result.compressedSize < result.originalSize);
}

TEST_CASE("CompressionWrapper - Compress random data", "[CompressionUtils][Compress]") {
    CompressionWrapper compressor;
    
    auto data = generateRandomData(1000);
    auto result = compressor.compress(data);
    
    REQUIRE(result.isSuccess());
    // Random data may not compress well, but should not fail
}

TEST_CASE("CompressionWrapper - Compress empty data", "[CompressionUtils][Compress]") {
    CompressionWrapper compressor;
    
    auto result = compressor.compress(nullptr, 0);
    
    REQUIRE_FALSE(result.isSuccess());
    REQUIRE(result.error == CompressionError::InvalidInput);
}

TEST_CASE("CompressionWrapper - Compress large data", "[CompressionUtils][Compress]") {
    CompressionWrapper compressor;
    
    // 1MB of compressible data
    auto data = generateCompressibleData(1024 * 1024);
    auto result = compressor.compress(data);
    
    REQUIRE(result.isSuccess());
    REQUIRE(result.originalSize == 1024 * 1024);
}

//============================================
// Decompression Tests
//============================================
TEST_CASE("CompressionWrapper - Decompress raw pointer", "[CompressionUtils][Decompress]") {
    CompressionWrapper compressor;
    
    std::string original = "Hello World! This is test data for decompression.";
    auto compressed = compressor.compress(original);
    REQUIRE(compressed.isSuccess());
    
    auto result = compressor.decompress(
        compressed.data.data(),
        compressed.data.size()
    );
    
    REQUIRE(result.isSuccess());
    REQUIRE(result.decompressedSize == original.size());
    
    std::string decompressed(result.data.begin(), result.data.end());
    REQUIRE(decompressed == original);
}

TEST_CASE("CompressionWrapper - Decompress vector", "[CompressionUtils][Decompress]") {
    CompressionWrapper compressor;
    
    std::vector<uint8_t> original = {'T', 'e', 's', 't', ' ', 'D', 'a', 't', 'a'};
    auto compressed = compressor.compress(original);
    REQUIRE(compressed.isSuccess());
    
    auto result = compressor.decompress(compressed.data);
    
    REQUIRE(result.isSuccess());
    REQUIRE(result.data == original);
}

TEST_CASE("CompressionWrapper - Decompress to string", "[CompressionUtils][Decompress]") {
    CompressionWrapper compressor;
    
    std::string original = "String to compress and decompress";
    auto compressed = compressor.compress(original);
    REQUIRE(compressed.isSuccess());
    
    auto result = compressor.decompressToString(
        compressed.data.data(),
        compressed.data.size()
    );
    
    REQUIRE(result.has_value());
    REQUIRE(*result == original);
}

TEST_CASE("CompressionWrapper - Decompress to pre-allocated buffer", "[CompressionUtils][Decompress]") {
    CompressionWrapper compressor;
    
    std::string original = "Pre-allocated buffer test";
    auto compressed = compressor.compress(original);
    REQUIRE(compressed.isSuccess());
    
    std::vector<uint8_t> output(original.size() * 2); // Extra space
    size_t decompressedSize = 0;
    
    auto error = compressor.decompressTo(
        compressed.data.data(),
        compressed.data.size(),
        output.data(),
        output.size(),
        decompressedSize
    );
    
    REQUIRE(error == CompressionError::Success);
    REQUIRE(decompressedSize == original.size());
    
    std::string decompressed(output.begin(), output.begin() + decompressedSize);
    REQUIRE(decompressed == original);
}

TEST_CASE("CompressionWrapper - Decompress with size hint", "[CompressionUtils][Decompress]") {
    CompressionWrapper compressor;
    
    std::string original = "Data with size hint";
    auto compressed = compressor.compress(original);
    REQUIRE(compressed.isSuccess());
    
    auto result = compressor.decompress(
        compressed.data.data(),
        compressed.data.size(),
        original.size() // Size hint
    );
    
    REQUIRE(result.isSuccess());
    REQUIRE(result.decompressedSize == original.size());
}

TEST_CASE("CompressionWrapper - Decompress empty data", "[CompressionUtils][Decompress]") {
    CompressionWrapper compressor;
    
    auto result = compressor.decompress(nullptr, 0);
    
    REQUIRE_FALSE(result.isSuccess());
    REQUIRE(result.error == CompressionError::InvalidInput);
}

TEST_CASE("CompressionWrapper - Decompress corrupted data", "[CompressionUtils][Decompress]") {
    CompressionWrapper compressor;
    
    std::vector<uint8_t> garbage = {0x00, 0x01, 0x02, 0x03, 0x04};
    auto result = compressor.decompress(garbage);
    
    REQUIRE_FALSE(result.isSuccess());
    // Should fail with decompression or corruption error
}

TEST_CASE("CompressionWrapper - Decompress large data", "[CompressionUtils][Decompress]") {
    CompressionWrapper compressor;
    
    // 1MB of data
    auto original = generateCompressibleData(1024 * 1024);
    auto compressed = compressor.compress(original);
    REQUIRE(compressed.isSuccess());
    
    auto result = compressor.decompress(compressed.data);
    
    REQUIRE(result.isSuccess());
    REQUIRE(result.data == original);
}

//============================================
// Round-trip Tests
//============================================
TEST_CASE("CompressionWrapper - Round-trip various sizes", "[CompressionUtils][RoundTrip]") {
    CompressionWrapper compressor;
    
    std::vector<size_t> sizes = {1, 10, 100, 1000, 10000, 100000};
    
    for (size_t size : sizes) {
        DYNAMIC_SECTION("Size: " << size) {
            auto original = generateRandomData(size);
            
            auto compressed = compressor.compress(original);
            REQUIRE(compressed.isSuccess());
            
            auto decompressed = compressor.decompress(compressed.data);
            REQUIRE(decompressed.isSuccess());
            
            REQUIRE(decompressed.data == original);
        }
    }
}

TEST_CASE("CompressionWrapper - Round-trip binary data", "[CompressionUtils][RoundTrip]") {
    CompressionWrapper compressor;
    
    // Data with all byte values
    std::vector<uint8_t> original(256);
    std::iota(original.begin(), original.end(), 0);
    
    auto compressed = compressor.compress(original);
    REQUIRE(compressed.isSuccess());
    
    auto decompressed = compressor.decompress(compressed.data);
    REQUIRE(decompressed.isSuccess());
    
    REQUIRE(decompressed.data == original);
}

TEST_CASE("CompressionWrapper - Round-trip with null bytes", "[CompressionUtils][RoundTrip]") {
    CompressionWrapper compressor;
    
    std::vector<uint8_t> original = {0x00, 0x00, 'H', 'i', 0x00, 0x00};
    
    auto compressed = compressor.compress(original);
    REQUIRE(compressed.isSuccess());
    
    auto decompressed = compressor.decompress(compressed.data);
    REQUIRE(decompressed.isSuccess());
    
    REQUIRE(decompressed.data == original);
}

//============================================
// Compression Level Tests
//============================================
TEST_CASE("CompressionWrapper - Different compression levels", "[CompressionUtils][Level]") {
    auto data = generateCompressibleData(10000);
    
    std::vector<CompressionLevel> levels = {
        CompressionLevel::Fastest,
        CompressionLevel::Fast,
        CompressionLevel::Default,
        CompressionLevel::Better,
        CompressionLevel::Best
    };
    
    std::vector<size_t> compressedSizes;
    
    for (auto level : levels) {
        CompressionWrapper compressor(CompressionFormat::Zstd, level);
        auto result = compressor.compress(data);
        REQUIRE(result.isSuccess());
        compressedSizes.push_back(result.compressedSize);
    }
    
    // Generally, higher levels should produce smaller output
    // But this is not guaranteed for all data
    REQUIRE(compressedSizes.size() == 5);
}

TEST_CASE("CompressionWrapper - Set and get level", "[CompressionUtils][Level]") {
    CompressionWrapper compressor(CompressionFormat::Zstd, CompressionLevel::Fast);
    
    REQUIRE(compressor.getLevel() == CompressionLevel::Fast);
    
    compressor.setLevel(CompressionLevel::Best);
    REQUIRE(compressor.getLevel() == CompressionLevel::Best);
}

TEST_CASE("CompressionWrapper - Level affects compression ratio", "[CompressionUtils][Level]") {
    auto data = generateCompressibleData(100000);
    
    CompressionWrapper fastCompressor(CompressionFormat::Zstd, CompressionLevel::Fastest);
    CompressionWrapper bestCompressor(CompressionFormat::Zstd, CompressionLevel::Best);
    
    auto fastResult = fastCompressor.compress(data);
    auto bestResult = bestCompressor.compress(data);
    
    REQUIRE(fastResult.isSuccess());
    REQUIRE(bestResult.isSuccess());
    
    // Best should generally produce smaller output for compressible data
    REQUIRE(bestResult.compressedSize <= fastResult.compressedSize);
}

//============================================
// Utility Method Tests
//============================================
TEST_CASE("CompressionWrapper - Get compress bound", "[CompressionUtils][Utility]") {
    CompressionWrapper compressor;
    size_t inputSize = 1000;
    size_t bound = compressor.getCompressBound(inputSize);
    
    // Bound should be at least input size
    REQUIRE(bound >= inputSize);
    
    // Verify bound is sufficient
    auto data = generateRandomData(inputSize);
    auto result = compressor.compress(data);
    
    REQUIRE(result.compressedSize <= bound);
}

TEST_CASE("CompressionWrapper - Get decompressed size", "[CompressionUtils][Utility]") {
    CompressionWrapper compressor;
    
    std::string original = "Test data for size detection";
    auto compressed = compressor.compress(original);
    REQUIRE(compressed.isSuccess());
    
    size_t size = compressor.getDecompressedSize(
        compressed.data.data(),
        compressed.data.size()
    );
    
    REQUIRE(size == original.size());
}

TEST_CASE("CompressionWrapper - Get backend name", "[CompressionUtils][Utility]") {
    CompressionWrapper compressor;
    std::string name = compressor.getBackendName();
    
    REQUIRE_FALSE(name.empty());
    REQUIRE(name == "zstd");
}

TEST_CASE("CompressionWrapper - Get backend version", "[CompressionUtils][Utility]") {
    CompressionWrapper compressor;
    std::string version = compressor.getBackendVersion();
    
    REQUIRE_FALSE(version.empty());
}

TEST_CASE("CompressionWrapper - Error to string", "[CompressionUtils][Utility]") {
    CompressionWrapper compressor;
    REQUIRE(std::string(compressor.errorToString(CompressionError::Success)) == "Success");
    REQUIRE(std::string(compressor.errorToString(CompressionError::InvalidInput)) == "Invalid input");
    REQUIRE(std::string(compressor.errorToString(CompressionError::CompressionFailed)) == "Compression failed");
    REQUIRE(std::string(compressor.errorToString(CompressionError::DecompressionFailed)) == "Decompression failed");
    REQUIRE(std::string(compressor.errorToString(CompressionError::CorruptedData)) == "Corrupted data");
    REQUIRE(std::string(compressor.errorToString(CompressionError::OutputBufferTooSmall)) == "Output buffer too small");
}

//============================================
// Convenience Function Tests
//============================================
TEST_CASE("Convenience function - compressData", "[CompressionUtils][Convenience]") {
    std::string data = "Quick compress test";
    
    auto result = compressData(
        reinterpret_cast<const uint8_t*>(data.data()),
        data.size()
    );
    
    REQUIRE(result.isSuccess());
    REQUIRE(result.originalSize == data.size());
}

TEST_CASE("Convenience function - compressData with level", "[CompressionUtils][Convenience]") {
    std::string data = "Quick compress with level";
    
    auto result = compressData(
        reinterpret_cast<const uint8_t*>(data.data()),
        data.size(),
        CompressionFormat::Zstd,
        CompressionLevel::Best
    );
    
    REQUIRE(result.isSuccess());
}

TEST_CASE("Convenience function - decompressData", "[CompressionUtils][Convenience]") {
    std::string original = "Quick decompress test";
    
    auto compressed = compressData(
        reinterpret_cast<const uint8_t*>(original.data()),
        original.size()
    );
    REQUIRE(compressed.isSuccess());
    
    auto decompressed = decompressData(
        compressed.data.data(),
        compressed.data.size()
    );
    
    REQUIRE(decompressed.isSuccess());
    
    std::string result(decompressed.data.begin(), decompressed.data.end());
    REQUIRE(result == original);
}

//============================================
// Error Handling Tests
//============================================
TEST_CASE("CompressionWrapper - Buffer too small error", "[CompressionUtils][Error]") {
    CompressionWrapper compressor;
    
    std::string data = "Data that needs more space than provided";
    
    std::vector<uint8_t> tinyBuffer(1);
    size_t compressedSize = 0;
    
    auto error = compressor.compressTo(
        reinterpret_cast<const uint8_t*>(data.data()),
        data.size(),
        tinyBuffer.data(),
        tinyBuffer.size(),
        compressedSize
    );
    
    REQUIRE(error == CompressionError::OutputBufferTooSmall);
}

TEST_CASE("CompressionWrapper - Decompress buffer too small", "[CompressionUtils][Error]") {
    CompressionWrapper compressor;
    
    std::string original = "Original data that is longer than output buffer";
    auto compressed = compressor.compress(original);
    REQUIRE(compressed.isSuccess());
    
    std::vector<uint8_t> tinyBuffer(1);
    size_t decompressedSize = 0;
    
    auto error = compressor.decompressTo(
        compressed.data.data(),
        compressed.data.size(),
        tinyBuffer.data(),
        tinyBuffer.size(),
        decompressedSize
    );
    
    REQUIRE(error == CompressionError::OutputBufferTooSmall);
}

//============================================
// Result Structure Tests
//============================================
TEST_CASE("CompressionResult - Boolean conversion", "[CompressionUtils][Result]") {
    CompressionResult success;
    success.error = CompressionError::Success;
    
    CompressionResult failure;
    failure.error = CompressionError::CompressionFailed;
    
    REQUIRE(static_cast<bool>(success) == true);
    REQUIRE(static_cast<bool>(failure) == false);
    
    if (success) {
        REQUIRE(true); // Should enter
    } else {
        REQUIRE(false); // Should not reach
    }
}

TEST_CASE("DecompressionResult - Boolean conversion", "[CompressionUtils][Result]") {
    DecompressionResult success;
    success.error = CompressionError::Success;
    
    DecompressionResult failure;
    failure.error = CompressionError::DecompressionFailed;
    
    REQUIRE(static_cast<bool>(success) == true);
    REQUIRE(static_cast<bool>(failure) == false);
}

//============================================
// Move Semantics Tests
//============================================
TEST_CASE("CompressionWrapper - Move constructor", "[CompressionUtils][Move]") {
    CompressionWrapper original(CompressionFormat::Zstd, CompressionLevel::Best);
    
    CompressionWrapper moved(std::move(original));
    
    REQUIRE(moved.getLevel() == CompressionLevel::Best);
    
    // Verify moved instance works
    std::string data = "Test after move";
    auto result = moved.compress(data);
    REQUIRE(result.isSuccess());
}

TEST_CASE("CompressionWrapper - Move assignment", "[CompressionUtils][Move]") {
    CompressionWrapper original(CompressionFormat::Zstd, CompressionLevel::Best);
    CompressionWrapper target(CompressionFormat::Zstd, CompressionLevel::Fastest);
    
    target = std::move(original);
    
    REQUIRE(target.getLevel() == CompressionLevel::Best);
    
    // Verify target works
    std::string data = "Test after move assignment";
    auto result = target.compress(data);
    REQUIRE(result.isSuccess());
}

//============================================
// Stress Tests
//============================================
TEST_CASE("CompressionWrapper - Multiple compressions", "[CompressionUtils][Stress]") {
    CompressionWrapper compressor;
    
    for (int i = 0; i < 100; ++i) {
        auto data = generateRandomData(1000 + i * 10);
        auto result = compressor.compress(data);
        REQUIRE(result.isSuccess());
    }
}

TEST_CASE("CompressionWrapper - Compress same data multiple times", "[CompressionUtils][Stress]") {
    CompressionWrapper compressor;
    
    auto data = generateCompressibleData(10000);
    std::vector<size_t> sizes;
    
    for (int i = 0; i < 10; ++i) {
        auto result = compressor.compress(data);
        REQUIRE(result.isSuccess());
        sizes.push_back(result.compressedSize);
    }
    
    // All results should be identical
    for (size_t size : sizes) {
        REQUIRE(size == sizes[0]);
    }
}

//============================================
// Gzip Tests
//============================================
TEST_CASE("CompressionWrapper - Gzip basic compression", "[CompressionUtils][Gzip]") {
    CompressionWrapper compressor(CompressionFormat::Gzip);
    
    std::string data = "Hello World! This is a test string for gzip compression.";
    auto result = compressor.compress(data);
    
    REQUIRE(result.isSuccess());
    REQUIRE(result.format == CompressionFormat::Gzip);
    REQUIRE(result.originalSize == data.size());
    REQUIRE(result.compressedSize > 0);
    
    // Verify gzip magic bytes (0x1F 0x8B)
    REQUIRE(result.data.size() >= 2);
    REQUIRE(result.data[0] == 0x1F);
    REQUIRE(result.data[1] == 0x8B);
}

TEST_CASE("CompressionWrapper - Gzip compression and decompression", "[CompressionUtils][Gzip]") {
    CompressionWrapper compressor(CompressionFormat::Gzip);
    
    std::string original = "This is test data for gzip compression and decompression.";
    auto compressed = compressor.compress(original);
    REQUIRE(compressed.isSuccess());
    
    auto decompressed = compressor.decompress(compressed.data);
    REQUIRE(decompressed.isSuccess());
    REQUIRE(decompressed.format == CompressionFormat::Gzip);
    
    std::string result(decompressed.data.begin(), decompressed.data.end());
    REQUIRE(result == original);
}

TEST_CASE("CompressionWrapper - Gzip large data", "[CompressionUtils][Gzip]") {
    CompressionWrapper compressor(CompressionFormat::Gzip);
    
    auto data = generateCompressibleData(100000);
    auto compressed = compressor.compress(data);
    REQUIRE(compressed.isSuccess());
    REQUIRE(compressed.ratio < 1.0);
    
    auto decompressed = compressor.decompress(compressed.data);
    REQUIRE(decompressed.isSuccess());
    REQUIRE(decompressed.data == data);
}

TEST_CASE("CompressionWrapper - Gzip getBackendName", "[CompressionUtils][Gzip]") {
    CompressionWrapper gzip(CompressionFormat::Gzip);
    CompressionWrapper zstd(CompressionFormat::Zstd);
    
    REQUIRE(gzip.getBackendName() == "zlib-ng");
    REQUIRE(zstd.getBackendName() == "zstd");
}

TEST_CASE("CompressionWrapper - Gzip formatToString", "[CompressionUtils][Gzip]") {
    CompressionWrapper compressor;
    
    REQUIRE(std::string(compressor.formatToString(CompressionFormat::Gzip)) == "gzip");
    REQUIRE(std::string(compressor.formatToString(CompressionFormat::Zstd)) == "zstd");
}

TEST_CASE("CompressionWrapper - Gzip compression levels", "[CompressionUtils][Gzip]") {
    std::string data = "This is test data for testing different gzip compression levels.";
    
    std::vector<CompressionLevel> levels = {
        CompressionLevel::Fastest,
        CompressionLevel::Fast,
        CompressionLevel::Default,
        CompressionLevel::Better,
        CompressionLevel::Best
    };
    
    for (auto level : levels) {
        CompressionWrapper compressor(CompressionFormat::Gzip, level);
        auto result = compressor.compress(data);
        REQUIRE(result.isSuccess());
        // Higher levels should generally produce smaller or equal output
        // (not strictly guaranteed for small data)
    }
}

//============================================
// Format Detection Tests
//============================================
TEST_CASE("CompressionWrapper - detectCompressionFormat Zstd", "[CompressionUtils][Detection]") {
    CompressionWrapper compressor(CompressionFormat::Zstd);
    
    std::string data = "Test data for format detection";
    auto compressed = compressor.compress(data);
    REQUIRE(compressed.isSuccess());
    
    auto format = detectCompressionFormat(compressed.data.data(), compressed.data.size());
    REQUIRE(format.has_value());
    REQUIRE(format.value() == CompressionFormat::Zstd);
}

TEST_CASE("CompressionWrapper - detectCompressionFormat Gzip", "[CompressionUtils][Detection]") {
    CompressionWrapper compressor(CompressionFormat::Gzip);
    
    std::string data = "Test data for format detection";
    auto compressed = compressor.compress(data);
    REQUIRE(compressed.isSuccess());
    
    auto format = detectCompressionFormat(compressed.data.data(), compressed.data.size());
    REQUIRE(format.has_value());
    REQUIRE(format.value() == CompressionFormat::Gzip);
}

TEST_CASE("CompressionWrapper - detectCompressionFormat unknown", "[CompressionUtils][Detection]") {
    std::vector<uint8_t> unknownData = {0x00, 0x01, 0x02, 0x03};
    
    auto format = detectCompressionFormat(unknownData.data(), unknownData.size());
    REQUIRE_FALSE(format.has_value());
}

TEST_CASE("CompressionWrapper - decompressDataAuto Zstd", "[CompressionUtils][Detection]") {
    std::string original = "Test data for auto decompression with Zstd";
    auto compressed = compressData(
        reinterpret_cast<const uint8_t*>(original.data()),
        original.size(),
        CompressionFormat::Zstd
    );
    REQUIRE(compressed.isSuccess());
    
    auto decompressed = decompressDataAuto(compressed.data.data(), compressed.data.size());
    REQUIRE(decompressed.isSuccess());
    REQUIRE(decompressed.format == CompressionFormat::Zstd);
    
    std::string result(decompressed.data.begin(), decompressed.data.end());
    REQUIRE(result == original);
}

TEST_CASE("CompressionWrapper - decompressDataAuto Gzip", "[CompressionUtils][Detection]") {
    std::string original = "Test data for auto decompression with Gzip";
    auto compressed = compressData(
        reinterpret_cast<const uint8_t*>(original.data()),
        original.size(),
        CompressionFormat::Gzip
    );
    REQUIRE(compressed.isSuccess());
    
    auto decompressed = decompressDataAuto(compressed.data.data(), compressed.data.size());
    REQUIRE(decompressed.isSuccess());
    REQUIRE(decompressed.format == CompressionFormat::Gzip);
    
    std::string result(decompressed.data.begin(), decompressed.data.end());
    REQUIRE(result == original);
}

//============================================
// Convenience Functions Tests (Updated)
//============================================
TEST_CASE("CompressionWrapper - compressData with format", "[CompressionUtils][Convenience]") {
    std::string data = "Test data for convenience function";
    
    auto zstdResult = compressData(
        reinterpret_cast<const uint8_t*>(data.data()),
        data.size(),
        CompressionFormat::Zstd
    );
    REQUIRE(zstdResult.isSuccess());
    REQUIRE(zstdResult.format == CompressionFormat::Zstd);
    
    auto gzipResult = compressData(
        reinterpret_cast<const uint8_t*>(data.data()),
        data.size(),
        CompressionFormat::Gzip
    );
    REQUIRE(gzipResult.isSuccess());
    REQUIRE(gzipResult.format == CompressionFormat::Gzip);
}

TEST_CASE("CompressionWrapper - Result contains format", "[CompressionUtils][Result]") {
    CompressionWrapper zstd(CompressionFormat::Zstd);
    CompressionWrapper gzip(CompressionFormat::Gzip);
    
    std::string data = "Test data";
    
    auto zstdResult = zstd.compress(data);
    REQUIRE(zstdResult.format == CompressionFormat::Zstd);
    
    auto gzipResult = gzip.compress(data);
    REQUIRE(gzipResult.format == CompressionFormat::Gzip);
}

