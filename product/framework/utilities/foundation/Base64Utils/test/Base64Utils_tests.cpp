#include <catch2/catch_test_macros.hpp>
#include "ucf/Utilities/Base64Utils/Base64Utils.h"

using namespace ucf::utilities;

//============================================
// Standard Base64 Encoding Tests
//============================================
TEST_CASE("Base64Utils - encode empty string", "[Base64Utils][Encode]") {
    auto result = Base64Utils::encode("");
    REQUIRE(result.isSuccess());
    REQUIRE(result.data.empty());
}

TEST_CASE("Base64Utils - encode simple string", "[Base64Utils][Encode]") {
    auto result = Base64Utils::encode("Hello");
    REQUIRE(result.isSuccess());
    REQUIRE(result.data == "SGVsbG8=");
}

TEST_CASE("Base64Utils - encode 'Man'", "[Base64Utils][Encode]") {
    auto result = Base64Utils::encode("Man");
    REQUIRE(result.isSuccess());
    REQUIRE(result.data == "TWFu");
}

TEST_CASE("Base64Utils - encode 'Ma'", "[Base64Utils][Encode]") {
    auto result = Base64Utils::encode("Ma");
    REQUIRE(result.isSuccess());
    REQUIRE(result.data == "TWE=");
}

TEST_CASE("Base64Utils - encode 'M'", "[Base64Utils][Encode]") {
    auto result = Base64Utils::encode("M");
    REQUIRE(result.isSuccess());
    REQUIRE(result.data == "TQ==");
}

TEST_CASE("Base64Utils - encode without padding", "[Base64Utils][Encode]") {
    auto result = Base64Utils::encode("Hello", Base64Variant::Standard, false);
    REQUIRE(result.isSuccess());
    REQUIRE(result.data == "SGVsbG8");
}

TEST_CASE("Base64Utils - encode binary data", "[Base64Utils][Encode]") {
    std::vector<uint8_t> data = {0x00, 0xFF, 0x10, 0x20};
    auto result = Base64Utils::encode(data);
    REQUIRE(result.isSuccess());
    REQUIRE(result.data == "AP8QIA==");
}

//============================================
// URL-safe Base64 Encoding Tests
//============================================
TEST_CASE("Base64Utils - encode URL-safe", "[Base64Utils][Encode][UrlSafe]") {
    // This string produces + and / in standard Base64
    std::vector<uint8_t> data = {0xfb, 0xff, 0xfe};
    
    auto stdResult = Base64Utils::encode(data, Base64Variant::Standard);
    REQUIRE(stdResult.isSuccess());
    REQUIRE(stdResult.data == "+//+");
    
    auto urlResult = Base64Utils::encode(data, Base64Variant::UrlSafe);
    REQUIRE(urlResult.isSuccess());
    REQUIRE(urlResult.data == "-__-");
}

//============================================
// Standard Base64 Decoding Tests
//============================================
TEST_CASE("Base64Utils - decode empty string", "[Base64Utils][Decode]") {
    auto result = Base64Utils::decode("");
    REQUIRE(result.isSuccess());
    REQUIRE(result.data.empty());
}

TEST_CASE("Base64Utils - decode simple string", "[Base64Utils][Decode]") {
    auto result = Base64Utils::decode("SGVsbG8=");
    REQUIRE(result.isSuccess());
    REQUIRE(std::string(result.data.begin(), result.data.end()) == "Hello");
}

TEST_CASE("Base64Utils - decode without padding", "[Base64Utils][Decode]") {
    auto result = Base64Utils::decode("SGVsbG8");
    REQUIRE(result.isSuccess());
    REQUIRE(std::string(result.data.begin(), result.data.end()) == "Hello");
}

TEST_CASE("Base64Utils - decode 'TWFu' to 'Man'", "[Base64Utils][Decode]") {
    auto result = Base64Utils::decode("TWFu");
    REQUIRE(result.isSuccess());
    REQUIRE(std::string(result.data.begin(), result.data.end()) == "Man");
}

TEST_CASE("Base64Utils - decodeToString", "[Base64Utils][Decode]") {
    auto result = Base64Utils::decodeToString("SGVsbG8gV29ybGQ=");
    REQUIRE(result.has_value());
    REQUIRE(result.value() == "Hello World");
}

TEST_CASE("Base64Utils - decode invalid character", "[Base64Utils][Decode]") {
    auto result = Base64Utils::decode("SGVs!G8=");
    REQUIRE_FALSE(result.isSuccess());
    REQUIRE_FALSE(result.errorMessage.empty());
}

TEST_CASE("Base64Utils - decode invalid padding", "[Base64Utils][Decode]") {
    auto result = Base64Utils::decode("SGVsbG8===");
    REQUIRE_FALSE(result.isSuccess());
}

//============================================
// URL-safe Base64 Decoding Tests
//============================================
TEST_CASE("Base64Utils - decode URL-safe", "[Base64Utils][Decode][UrlSafe]") {
    auto result = Base64Utils::decode("-__-", Base64Variant::UrlSafe);
    REQUIRE(result.isSuccess());
    REQUIRE(result.data.size() == 3);
    REQUIRE(result.data[0] == 0xfb);
    REQUIRE(result.data[1] == 0xff);
    REQUIRE(result.data[2] == 0xfe);
}

TEST_CASE("Base64Utils - decodeAuto standard", "[Base64Utils][Decode][Auto]") {
    auto result = Base64Utils::decodeAuto("+//+");
    REQUIRE(result.isSuccess());
    REQUIRE(result.variant == Base64Variant::Standard);
}

TEST_CASE("Base64Utils - decodeAuto URL-safe", "[Base64Utils][Decode][Auto]") {
    auto result = Base64Utils::decodeAuto("-__-");
    REQUIRE(result.isSuccess());
    REQUIRE(result.variant == Base64Variant::UrlSafe);
}

//============================================
// Round-trip Tests
//============================================
TEST_CASE("Base64Utils - roundtrip standard", "[Base64Utils][Roundtrip]") {
    std::string original = "The quick brown fox jumps over the lazy dog";
    
    auto encoded = Base64Utils::encode(original);
    REQUIRE(encoded.isSuccess());
    
    auto decoded = Base64Utils::decodeToString(encoded.data);
    REQUIRE(decoded.has_value());
    REQUIRE(decoded.value() == original);
}

TEST_CASE("Base64Utils - roundtrip URL-safe", "[Base64Utils][Roundtrip]") {
    std::vector<uint8_t> original;
    for (int i = 0; i < 256; ++i) {
        original.push_back(static_cast<uint8_t>(i));
    }
    
    auto encoded = Base64Utils::encode(original, Base64Variant::UrlSafe);
    REQUIRE(encoded.isSuccess());
    
    auto decoded = Base64Utils::decode(encoded.data, Base64Variant::UrlSafe);
    REQUIRE(decoded.isSuccess());
    REQUIRE(decoded.data == original);
}

TEST_CASE("Base64Utils - roundtrip without padding", "[Base64Utils][Roundtrip]") {
    std::string original = "Test without padding!";
    
    auto encoded = Base64Utils::encode(original, Base64Variant::Standard, false);
    REQUIRE(encoded.isSuccess());
    REQUIRE(encoded.data.find('=') == std::string::npos);
    
    auto decoded = Base64Utils::decodeToString(encoded.data);
    REQUIRE(decoded.has_value());
    REQUIRE(decoded.value() == original);
}

//============================================
// Utility Method Tests
//============================================
TEST_CASE("Base64Utils - detectVariant", "[Base64Utils][Utility]") {
    REQUIRE(Base64Utils::detectVariant("+abc") == Base64Variant::Standard);
    REQUIRE(Base64Utils::detectVariant("abc/def") == Base64Variant::Standard);
    REQUIRE(Base64Utils::detectVariant("-abc") == Base64Variant::UrlSafe);
    REQUIRE(Base64Utils::detectVariant("abc_def") == Base64Variant::UrlSafe);
    REQUIRE(Base64Utils::detectVariant("abcdef") == Base64Variant::Standard); // default
}

TEST_CASE("Base64Utils - isValid", "[Base64Utils][Utility]") {
    REQUIRE(Base64Utils::isValid("SGVsbG8="));
    REQUIRE(Base64Utils::isValid("SGVsbG8"));  // without padding
    REQUIRE(Base64Utils::isValid("TWFu"));
    REQUIRE(Base64Utils::isValid(""));
    
    REQUIRE_FALSE(Base64Utils::isValid("SGVs!G8="));  // invalid char
    REQUIRE_FALSE(Base64Utils::isValid("A"));         // invalid length
    REQUIRE_FALSE(Base64Utils::isValid("SGVsbG8==="));// too much padding
}

TEST_CASE("Base64Utils - getEncodedSize", "[Base64Utils][Utility]") {
    REQUIRE(Base64Utils::getEncodedSize(0) == 0);
    REQUIRE(Base64Utils::getEncodedSize(1) == 4);
    REQUIRE(Base64Utils::getEncodedSize(2) == 4);
    REQUIRE(Base64Utils::getEncodedSize(3) == 4);
    REQUIRE(Base64Utils::getEncodedSize(4) == 8);
    
    // Without padding
    REQUIRE(Base64Utils::getEncodedSize(1, false) == 2);
    REQUIRE(Base64Utils::getEncodedSize(2, false) == 3);
    REQUIRE(Base64Utils::getEncodedSize(3, false) == 4);
}

TEST_CASE("Base64Utils - getMaxDecodedSize", "[Base64Utils][Utility]") {
    REQUIRE(Base64Utils::getMaxDecodedSize("") == 0);
    REQUIRE(Base64Utils::getMaxDecodedSize("AAAA") == 3);
    REQUIRE(Base64Utils::getMaxDecodedSize("AAA=") == 2);
    REQUIRE(Base64Utils::getMaxDecodedSize("AA==") == 1);
}

TEST_CASE("Base64Utils - variantToString", "[Base64Utils][Utility]") {
    REQUIRE(Base64Utils::variantToString(Base64Variant::Standard) == "Standard");
    REQUIRE(Base64Utils::variantToString(Base64Variant::UrlSafe) == "UrlSafe");
}

//============================================
// RFC 4648 Test Vectors
//============================================
TEST_CASE("Base64Utils - RFC 4648 test vectors", "[Base64Utils][RFC]") {
    SECTION("Empty") {
        auto e = Base64Utils::encode("");
        REQUIRE(e.data == "");
    }
    SECTION("f") {
        auto e = Base64Utils::encode("f");
        REQUIRE(e.data == "Zg==");
    }
    SECTION("fo") {
        auto e = Base64Utils::encode("fo");
        REQUIRE(e.data == "Zm8=");
    }
    SECTION("foo") {
        auto e = Base64Utils::encode("foo");
        REQUIRE(e.data == "Zm9v");
    }
    SECTION("foob") {
        auto e = Base64Utils::encode("foob");
        REQUIRE(e.data == "Zm9vYg==");
    }
    SECTION("fooba") {
        auto e = Base64Utils::encode("fooba");
        REQUIRE(e.data == "Zm9vYmE=");
    }
    SECTION("foobar") {
        auto e = Base64Utils::encode("foobar");
        REQUIRE(e.data == "Zm9vYmFy");
    }
}
