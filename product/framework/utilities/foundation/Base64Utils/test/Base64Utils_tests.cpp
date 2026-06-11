#include <catch2/catch_test_macros.hpp>
#include "ucf/Utilities/Base64Utils/Base64Utils.h"

//============================================
// Standard Base64 Encoding Tests
//============================================
TEST_CASE("Base64Utils - encode empty string", "[Base64Utils][Encode]") {
    auto result = ucf::utilities::Base64Utils::encode("");
    REQUIRE(result.isSuccess());
    REQUIRE(result.data.empty());
}

TEST_CASE("Base64Utils - encode simple string", "[Base64Utils][Encode]") {
    auto result = ucf::utilities::Base64Utils::encode("Hello");
    REQUIRE(result.isSuccess());
    REQUIRE(result.data == "SGVsbG8=");
}

TEST_CASE("Base64Utils - encode 'Man'", "[Base64Utils][Encode]") {
    auto result = ucf::utilities::Base64Utils::encode("Man");
    REQUIRE(result.isSuccess());
    REQUIRE(result.data == "TWFu");
}

TEST_CASE("Base64Utils - encode 'Ma'", "[Base64Utils][Encode]") {
    auto result = ucf::utilities::Base64Utils::encode("Ma");
    REQUIRE(result.isSuccess());
    REQUIRE(result.data == "TWE=");
}

TEST_CASE("Base64Utils - encode 'M'", "[Base64Utils][Encode]") {
    auto result = ucf::utilities::Base64Utils::encode("M");
    REQUIRE(result.isSuccess());
    REQUIRE(result.data == "TQ==");
}

TEST_CASE("Base64Utils - encode without padding", "[Base64Utils][Encode]") {
    auto result = ucf::utilities::Base64Utils::encode("Hello", ucf::utilities::Base64Variant::Standard, false);
    REQUIRE(result.isSuccess());
    REQUIRE(result.data == "SGVsbG8");
}

TEST_CASE("Base64Utils - encode binary data", "[Base64Utils][Encode]") {
    std::vector<uint8_t> data = {0x00, 0xFF, 0x10, 0x20};
    auto result = ucf::utilities::Base64Utils::encode(data);
    REQUIRE(result.isSuccess());
    REQUIRE(result.data == "AP8QIA==");
}

//============================================
// URL-safe Base64 Encoding Tests
//============================================
TEST_CASE("Base64Utils - encode URL-safe", "[Base64Utils][Encode][UrlSafe]") {
    // This string produces + and / in standard Base64
    std::vector<uint8_t> data = {0xfb, 0xff, 0xfe};

    auto stdResult = ucf::utilities::Base64Utils::encode(data, ucf::utilities::Base64Variant::Standard);
    REQUIRE(stdResult.isSuccess());
    REQUIRE(stdResult.data == "+//+");

    auto urlResult = ucf::utilities::Base64Utils::encode(data, ucf::utilities::Base64Variant::UrlSafe);
    REQUIRE(urlResult.isSuccess());
    REQUIRE(urlResult.data == "-__-");
}

//============================================
// Standard Base64 Decoding Tests
//============================================
TEST_CASE("Base64Utils - decode empty string", "[Base64Utils][Decode]") {
    auto result = ucf::utilities::Base64Utils::decode("");
    REQUIRE(result.isSuccess());
    REQUIRE(result.data.empty());
}

TEST_CASE("Base64Utils - decode simple string", "[Base64Utils][Decode]") {
    auto result = ucf::utilities::Base64Utils::decode("SGVsbG8=");
    REQUIRE(result.isSuccess());
    REQUIRE(std::string(result.data.begin(), result.data.end()) == "Hello");
}

TEST_CASE("Base64Utils - decode without padding", "[Base64Utils][Decode]") {
    auto result = ucf::utilities::Base64Utils::decode("SGVsbG8");
    REQUIRE(result.isSuccess());
    REQUIRE(std::string(result.data.begin(), result.data.end()) == "Hello");
}

TEST_CASE("Base64Utils - decode 'TWFu' to 'Man'", "[Base64Utils][Decode]") {
    auto result = ucf::utilities::Base64Utils::decode("TWFu");
    REQUIRE(result.isSuccess());
    REQUIRE(std::string(result.data.begin(), result.data.end()) == "Man");
}

TEST_CASE("Base64Utils - decodeToString", "[Base64Utils][Decode]") {
    auto result = ucf::utilities::Base64Utils::decodeToString("SGVsbG8gV29ybGQ=");
    REQUIRE(result.has_value());
    REQUIRE(result.value() == "Hello World");
}

TEST_CASE("Base64Utils - decode invalid character", "[Base64Utils][Decode]") {
    auto result = ucf::utilities::Base64Utils::decode("SGVs!G8=");
    REQUIRE_FALSE(result.isSuccess());
    REQUIRE_FALSE(result.errorMessage.empty());
}

TEST_CASE("Base64Utils - decode invalid padding", "[Base64Utils][Decode]") {
    auto result = ucf::utilities::Base64Utils::decode("SGVsbG8===");
    REQUIRE_FALSE(result.isSuccess());
}

//============================================
// URL-safe Base64 Decoding Tests
//============================================
TEST_CASE("Base64Utils - decode URL-safe", "[Base64Utils][Decode][UrlSafe]") {
    auto result = ucf::utilities::Base64Utils::decode("-__-", ucf::utilities::Base64Variant::UrlSafe);
    REQUIRE(result.isSuccess());
    REQUIRE(result.data.size() == 3);
    REQUIRE(result.data[0] == 0xfb);
    REQUIRE(result.data[1] == 0xff);
    REQUIRE(result.data[2] == 0xfe);
}

TEST_CASE("Base64Utils - decodeAuto standard", "[Base64Utils][Decode][Auto]") {
    auto result = ucf::utilities::Base64Utils::decodeAuto("+//+");
    REQUIRE(result.isSuccess());
    REQUIRE(result.variant == ucf::utilities::Base64Variant::Standard);
}

TEST_CASE("Base64Utils - decodeAuto URL-safe", "[Base64Utils][Decode][Auto]") {
    auto result = ucf::utilities::Base64Utils::decodeAuto("-__-");
    REQUIRE(result.isSuccess());
    REQUIRE(result.variant == ucf::utilities::Base64Variant::UrlSafe);
}

//============================================
// Round-trip Tests
//============================================
TEST_CASE("Base64Utils - roundtrip standard", "[Base64Utils][Roundtrip]") {
    std::string original = "The quick brown fox jumps over the lazy dog";

    auto encoded = ucf::utilities::Base64Utils::encode(original);
    REQUIRE(encoded.isSuccess());

    auto decoded = ucf::utilities::Base64Utils::decodeToString(encoded.data);
    REQUIRE(decoded.has_value());
    REQUIRE(decoded.value() == original);
}

TEST_CASE("Base64Utils - roundtrip URL-safe", "[Base64Utils][Roundtrip]") {
    std::vector<uint8_t> original;
    for (int i = 0; i < 256; ++i) {
        original.push_back(static_cast<uint8_t>(i));
    }

    auto encoded = ucf::utilities::Base64Utils::encode(original, ucf::utilities::Base64Variant::UrlSafe);
    REQUIRE(encoded.isSuccess());

    auto decoded = ucf::utilities::Base64Utils::decode(encoded.data, ucf::utilities::Base64Variant::UrlSafe);
    REQUIRE(decoded.isSuccess());
    REQUIRE(decoded.data == original);
}

TEST_CASE("Base64Utils - roundtrip without padding", "[Base64Utils][Roundtrip]") {
    std::string original = "Test without padding!";

    auto encoded = ucf::utilities::Base64Utils::encode(original, ucf::utilities::Base64Variant::Standard, false);
    REQUIRE(encoded.isSuccess());
    REQUIRE(encoded.data.find('=') == std::string::npos);

    auto decoded = ucf::utilities::Base64Utils::decodeToString(encoded.data);
    REQUIRE(decoded.has_value());
    REQUIRE(decoded.value() == original);
}

//============================================
// Utility Method Tests
//============================================
TEST_CASE("Base64Utils - detectVariant", "[Base64Utils][Utility]") {
    REQUIRE(ucf::utilities::Base64Utils::detectVariant("+abc") == ucf::utilities::Base64Variant::Standard);
    REQUIRE(ucf::utilities::Base64Utils::detectVariant("abc/def") == ucf::utilities::Base64Variant::Standard);
    REQUIRE(ucf::utilities::Base64Utils::detectVariant("-abc") == ucf::utilities::Base64Variant::UrlSafe);
    REQUIRE(ucf::utilities::Base64Utils::detectVariant("abc_def") == ucf::utilities::Base64Variant::UrlSafe);
    REQUIRE(ucf::utilities::Base64Utils::detectVariant("abcdef") == ucf::utilities::Base64Variant::Standard); // default
}

TEST_CASE("Base64Utils - isValid", "[Base64Utils][Utility]") {
    REQUIRE(ucf::utilities::Base64Utils::isValid("SGVsbG8="));
    REQUIRE(ucf::utilities::Base64Utils::isValid("SGVsbG8"));  // without padding
    REQUIRE(ucf::utilities::Base64Utils::isValid("TWFu"));
    REQUIRE(ucf::utilities::Base64Utils::isValid(""));

    REQUIRE_FALSE(ucf::utilities::Base64Utils::isValid("SGVs!G8="));  // invalid char
    REQUIRE_FALSE(ucf::utilities::Base64Utils::isValid("A"));         // invalid length
    REQUIRE_FALSE(ucf::utilities::Base64Utils::isValid("SGVsbG8==="));// too much padding
}

TEST_CASE("Base64Utils - getEncodedSize", "[Base64Utils][Utility]") {
    REQUIRE(ucf::utilities::Base64Utils::getEncodedSize(0) == 0);
    REQUIRE(ucf::utilities::Base64Utils::getEncodedSize(1) == 4);
    REQUIRE(ucf::utilities::Base64Utils::getEncodedSize(2) == 4);
    REQUIRE(ucf::utilities::Base64Utils::getEncodedSize(3) == 4);
    REQUIRE(ucf::utilities::Base64Utils::getEncodedSize(4) == 8);

    // Without padding
    REQUIRE(ucf::utilities::Base64Utils::getEncodedSize(1, false) == 2);
    REQUIRE(ucf::utilities::Base64Utils::getEncodedSize(2, false) == 3);
    REQUIRE(ucf::utilities::Base64Utils::getEncodedSize(3, false) == 4);
}

TEST_CASE("Base64Utils - getMaxDecodedSize", "[Base64Utils][Utility]") {
    REQUIRE(ucf::utilities::Base64Utils::getMaxDecodedSize("") == 0);
    REQUIRE(ucf::utilities::Base64Utils::getMaxDecodedSize("AAAA") == 3);
    REQUIRE(ucf::utilities::Base64Utils::getMaxDecodedSize("AAA=") == 2);
    REQUIRE(ucf::utilities::Base64Utils::getMaxDecodedSize("AA==") == 1);
}

TEST_CASE("Base64Utils - variantToString", "[Base64Utils][Utility]") {
    REQUIRE(ucf::utilities::Base64Utils::variantToString(ucf::utilities::Base64Variant::Standard) == "Standard");
    REQUIRE(ucf::utilities::Base64Utils::variantToString(ucf::utilities::Base64Variant::UrlSafe) == "UrlSafe");
}

//============================================
// RFC 4648 Test Vectors
//============================================
TEST_CASE("Base64Utils - RFC 4648 test vectors", "[Base64Utils][RFC]") {
    SECTION("Empty") {
        auto e = ucf::utilities::Base64Utils::encode("");
        REQUIRE(e.data == "");
    }
    SECTION("f") {
        auto e = ucf::utilities::Base64Utils::encode("f");
        REQUIRE(e.data == "Zg==");
    }
    SECTION("fo") {
        auto e = ucf::utilities::Base64Utils::encode("fo");
        REQUIRE(e.data == "Zm8=");
    }
    SECTION("foo") {
        auto e = ucf::utilities::Base64Utils::encode("foo");
        REQUIRE(e.data == "Zm9v");
    }
    SECTION("foob") {
        auto e = ucf::utilities::Base64Utils::encode("foob");
        REQUIRE(e.data == "Zm9vYg==");
    }
    SECTION("fooba") {
        auto e = ucf::utilities::Base64Utils::encode("fooba");
        REQUIRE(e.data == "Zm9vYmE=");
    }
    SECTION("foobar") {
        auto e = ucf::utilities::Base64Utils::encode("foobar");
        REQUIRE(e.data == "Zm9vYmFy");
    }
}
