#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

namespace ucf::utilities {

/**
 * @brief Base64 encoding variant
 */
enum class Base64Variant {
    Standard,   ///< Standard Base64 (A-Za-z0-9+/) with = padding
    UrlSafe     ///< URL-safe Base64 (A-Za-z0-9-_)
};

/**
 * @brief Result of Base64 encoding operation
 */
struct Utilities_EXPORT Base64EncodeResult {
    bool success = false;
    std::string data;
    Base64Variant variant = Base64Variant::Standard;
    std::string errorMessage;

    [[nodiscard]] bool isSuccess() const noexcept { return success; }
    [[nodiscard]] explicit operator bool() const noexcept { return success; }
};

/**
 * @brief Result of Base64 decoding operation
 */
struct Utilities_EXPORT Base64DecodeResult {
    bool success = false;
    std::vector<uint8_t> data;
    Base64Variant variant = Base64Variant::Standard;
    std::string errorMessage;

    [[nodiscard]] bool isSuccess() const noexcept { return success; }
    [[nodiscard]] explicit operator bool() const noexcept { return success; }
};

/**
 * @brief Base64 encoding and decoding utilities
 * 
 * Supports both standard Base64 (RFC 4648) and URL-safe variant.
 */
class Utilities_EXPORT Base64Utils {
public:
    //========================================
    // Encoding
    //========================================

    /**
     * @brief Encode binary data to Base64
     * @param data Pointer to data
     * @param size Size of data in bytes
     * @param variant Base64 variant to use
     * @param withPadding Whether to include '=' padding
     * @return Encoding result
     */
    static Base64EncodeResult encode(const void* data, size_t size,
                                     Base64Variant variant = Base64Variant::Standard,
                                     bool withPadding = true);

    /**
     * @brief Encode binary data to Base64
     */
    static Base64EncodeResult encode(const std::vector<uint8_t>& data,
                                     Base64Variant variant = Base64Variant::Standard,
                                     bool withPadding = true);

    /**
     * @brief Encode string to Base64
     */
    static Base64EncodeResult encode(std::string_view text,
                                     Base64Variant variant = Base64Variant::Standard,
                                     bool withPadding = true);

    //========================================
    // Decoding
    //========================================

    /**
     * @brief Decode Base64 string to binary data
     * @param encoded Base64 encoded string
     * @param variant Base64 variant to use
     * @return Decoding result with binary data
     */
    static Base64DecodeResult decode(std::string_view encoded,
                                     Base64Variant variant = Base64Variant::Standard);

    /**
     * @brief Decode Base64 string to text string
     * @return nullopt if decoding fails
     */
    static std::optional<std::string> decodeToString(std::string_view encoded,
                                                     Base64Variant variant = Base64Variant::Standard);

    /**
     * @brief Auto-detect variant and decode
     */
    static Base64DecodeResult decodeAuto(std::string_view encoded);

    //========================================
    // Utility Methods
    //========================================

    /**
     * @brief Detect which Base64 variant the string uses
     */
    static Base64Variant detectVariant(std::string_view encoded);

    /**
     * @brief Check if string is valid Base64
     */
    static bool isValid(std::string_view encoded, 
                        Base64Variant variant = Base64Variant::Standard);

    /**
     * @brief Calculate encoded size for given input size
     */
    static size_t getEncodedSize(size_t inputSize, bool withPadding = true);

    /**
     * @brief Calculate maximum decoded size for given encoded string
     */
    static size_t getMaxDecodedSize(std::string_view encoded);

    /**
     * @brief Convert variant to string name
     */
    static std::string_view variantToString(Base64Variant variant);

private:
    static const char* getAlphabet(Base64Variant variant);
    static const int8_t* getDecodeTable(Base64Variant variant);
};

} // namespace ucf::utilities
