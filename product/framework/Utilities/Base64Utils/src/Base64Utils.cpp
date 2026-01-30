#include "ucf/Utilities/Base64Utils/Base64Utils.h"

namespace ucf::utilities {

namespace {

// Standard Base64 alphabet
constexpr char kStandardAlphabet[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// URL-safe Base64 alphabet
constexpr char kUrlSafeAlphabet[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

// Decode table for standard Base64 (-1 = invalid, -2 = padding '=')
constexpr int8_t kStandardDecodeTable[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // 0-15
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // 16-31
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  // 32-47  (+, /)
    52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-2,-1,-1,  // 48-63  (0-9, =)
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  // 64-79  (A-O)
    15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  // 80-95  (P-Z)
    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  // 96-111 (a-o)
    41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  // 112-127 (p-z)
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // 128-143
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // 144-159
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // 160-175
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // 176-191
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // 192-207
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // 208-223
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // 224-239
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   // 240-255
};

// Decode table for URL-safe Base64
constexpr int8_t kUrlSafeDecodeTable[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // 0-15
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // 16-31
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,  // 32-47  (-)
    52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-2,-1,-1,  // 48-63  (0-9, =)
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  // 64-79  (A-O)
    15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,63,  // 80-95  (P-Z, _)
    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  // 96-111 (a-o)
    41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  // 112-127 (p-z)
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // 128-143
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // 144-159
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // 160-175
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // 176-191
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // 192-207
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // 208-223
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // 224-239
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   // 240-255
};

} // anonymous namespace

const char* Base64Utils::getAlphabet(Base64Variant variant) {
    return variant == Base64Variant::UrlSafe ? kUrlSafeAlphabet : kStandardAlphabet;
}

const int8_t* Base64Utils::getDecodeTable(Base64Variant variant) {
    return variant == Base64Variant::UrlSafe ? kUrlSafeDecodeTable : kStandardDecodeTable;
}

Base64EncodeResult Base64Utils::encode(const void* data, size_t size,
                                       Base64Variant variant, bool withPadding) {
    Base64EncodeResult result;
    result.variant = variant;

    if (data == nullptr && size > 0) {
        result.errorMessage = "Null data pointer with non-zero size";
        return result;
    }

    if (size == 0) {
        result.success = true;
        result.data = "";
        return result;
    }

    const auto* input = static_cast<const uint8_t*>(data);
    const char* alphabet = getAlphabet(variant);

    size_t outputSize = getEncodedSize(size, withPadding);
    result.data.reserve(outputSize);

    size_t i = 0;
    // Process 3 bytes at a time
    for (; i + 2 < size; i += 3) {
        uint32_t triple = (static_cast<uint32_t>(input[i]) << 16) |
                          (static_cast<uint32_t>(input[i + 1]) << 8) |
                          static_cast<uint32_t>(input[i + 2]);
        
        result.data.push_back(alphabet[(triple >> 18) & 0x3F]);
        result.data.push_back(alphabet[(triple >> 12) & 0x3F]);
        result.data.push_back(alphabet[(triple >> 6) & 0x3F]);
        result.data.push_back(alphabet[triple & 0x3F]);
    }

    // Handle remaining bytes
    size_t remaining = size - i;
    if (remaining == 1) {
        uint32_t val = static_cast<uint32_t>(input[i]) << 16;
        result.data.push_back(alphabet[(val >> 18) & 0x3F]);
        result.data.push_back(alphabet[(val >> 12) & 0x3F]);
        if (withPadding) {
            result.data.push_back('=');
            result.data.push_back('=');
        }
    } else if (remaining == 2) {
        uint32_t val = (static_cast<uint32_t>(input[i]) << 16) |
                       (static_cast<uint32_t>(input[i + 1]) << 8);
        result.data.push_back(alphabet[(val >> 18) & 0x3F]);
        result.data.push_back(alphabet[(val >> 12) & 0x3F]);
        result.data.push_back(alphabet[(val >> 6) & 0x3F]);
        if (withPadding) {
            result.data.push_back('=');
        }
    }

    result.success = true;
    return result;
}

Base64EncodeResult Base64Utils::encode(const std::vector<uint8_t>& data,
                                       Base64Variant variant, bool withPadding) {
    return encode(data.data(), data.size(), variant, withPadding);
}

Base64EncodeResult Base64Utils::encode(std::string_view text,
                                       Base64Variant variant, bool withPadding) {
    return encode(text.data(), text.size(), variant, withPadding);
}

Base64DecodeResult Base64Utils::decode(std::string_view encoded, Base64Variant variant) {
    Base64DecodeResult result;
    result.variant = variant;

    if (encoded.empty()) {
        result.success = true;
        return result;
    }

    const int8_t* decodeTable = getDecodeTable(variant);
    
    // Remove trailing padding for size calculation
    size_t inputLen = encoded.size();
    while (inputLen > 0 && encoded[inputLen - 1] == '=') {
        --inputLen;
    }

    // Validate and count padding
    size_t paddingCount = encoded.size() - inputLen;
    if (paddingCount > 2) {
        result.errorMessage = "Invalid padding";
        return result;
    }

    // Calculate output size
    size_t outputSize = (inputLen * 3) / 4;
    result.data.reserve(outputSize);

    uint32_t buffer = 0;
    int bitsCollected = 0;

    for (size_t i = 0; i < inputLen; ++i) {
        uint8_t ch = static_cast<uint8_t>(encoded[i]);
        int8_t value = decodeTable[ch];

        if (value == -1) {
            result.errorMessage = "Invalid character at position " + std::to_string(i);
            result.data.clear();
            return result;
        }
        if (value == -2) {
            // Padding in middle of string
            result.errorMessage = "Unexpected padding at position " + std::to_string(i);
            result.data.clear();
            return result;
        }

        buffer = (buffer << 6) | static_cast<uint32_t>(value);
        bitsCollected += 6;

        if (bitsCollected >= 8) {
            bitsCollected -= 8;
            result.data.push_back(static_cast<uint8_t>((buffer >> bitsCollected) & 0xFF));
        }
    }

    result.success = true;
    return result;
}

std::optional<std::string> Base64Utils::decodeToString(std::string_view encoded, 
                                                        Base64Variant variant) {
    auto result = decode(encoded, variant);
    if (!result.success) {
        return std::nullopt;
    }
    return std::string(result.data.begin(), result.data.end());
}

Base64DecodeResult Base64Utils::decodeAuto(std::string_view encoded) {
    Base64Variant variant = detectVariant(encoded);
    return decode(encoded, variant);
}

Base64Variant Base64Utils::detectVariant(std::string_view encoded) {
    for (char c : encoded) {
        if (c == '+' || c == '/') {
            return Base64Variant::Standard;
        }
        if (c == '-' || c == '_') {
            return Base64Variant::UrlSafe;
        }
    }
    // Default to Standard if no distinguishing characters found
    return Base64Variant::Standard;
}

bool Base64Utils::isValid(std::string_view encoded, Base64Variant variant) {
    if (encoded.empty()) {
        return true;
    }

    const int8_t* decodeTable = getDecodeTable(variant);
    bool foundPadding = false;
    size_t paddingCount = 0;

    for (size_t i = 0; i < encoded.size(); ++i) {
        uint8_t ch = static_cast<uint8_t>(encoded[i]);
        int8_t value = decodeTable[ch];

        if (value == -2) {  // Padding
            foundPadding = true;
            ++paddingCount;
            if (paddingCount > 2) {
                return false;
            }
        } else if (value == -1) {
            return false;  // Invalid character
        } else if (foundPadding) {
            return false;  // Non-padding after padding
        }
    }

    // Check valid length (must be multiple of 4 with padding, or valid without)
    size_t dataLen = encoded.size() - paddingCount;
    if (paddingCount > 0) {
        return (encoded.size() % 4) == 0;
    }
    
    // Without padding, length mod 4 cannot be 1
    return (dataLen % 4) != 1;
}

size_t Base64Utils::getEncodedSize(size_t inputSize, bool withPadding) {
    if (inputSize == 0) {
        return 0;
    }
    
    if (withPadding) {
        return ((inputSize + 2) / 3) * 4;
    } else {
        size_t fullGroups = inputSize / 3;
        size_t remaining = inputSize % 3;
        return fullGroups * 4 + (remaining == 0 ? 0 : remaining + 1);
    }
}

size_t Base64Utils::getMaxDecodedSize(std::string_view encoded) {
    if (encoded.empty()) {
        return 0;
    }
    
    size_t len = encoded.size();
    // Remove padding from calculation
    while (len > 0 && encoded[len - 1] == '=') {
        --len;
    }
    
    return (len * 3) / 4;
}

std::string_view Base64Utils::variantToString(Base64Variant variant) {
    switch (variant) {
        case Base64Variant::Standard: return "Standard";
        case Base64Variant::UrlSafe:  return "UrlSafe";
    }
    return "Unknown";
}

} // namespace ucf::utilities
