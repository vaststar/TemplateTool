#include <catch2/catch_test_macros.hpp>
#include <ucf/Utilities/StringUtils/StringUtils.h>

#include <string>

using namespace ucf::utilities;

//============================================
// Trim Tests - Basic
//============================================
TEST_CASE("StringUtils::trim - No whitespace", "[StringUtils][Trim]") {
    REQUIRE(StringUtils::trim("hello") == "hello");
    REQUIRE(StringUtils::trim("world") == "world");
    REQUIRE(StringUtils::trim("test123") == "test123");
}

TEST_CASE("StringUtils::trim - Leading whitespace", "[StringUtils][Trim]") {
    REQUIRE(StringUtils::trim("  hello") == "hello");
    REQUIRE(StringUtils::trim("   world") == "world");
    REQUIRE(StringUtils::trim("\thello") == "hello");
    REQUIRE(StringUtils::trim("\nhello") == "hello");
    REQUIRE(StringUtils::trim("\r\nhello") == "hello");
}

TEST_CASE("StringUtils::trim - Trailing whitespace", "[StringUtils][Trim]") {
    REQUIRE(StringUtils::trim("hello  ") == "hello");
    REQUIRE(StringUtils::trim("world   ") == "world");
    REQUIRE(StringUtils::trim("hello\t") == "hello");
    REQUIRE(StringUtils::trim("hello\n") == "hello");
    REQUIRE(StringUtils::trim("hello\r\n") == "hello");
}

TEST_CASE("StringUtils::trim - Both sides whitespace", "[StringUtils][Trim]") {
    REQUIRE(StringUtils::trim("  hello  ") == "hello");
    REQUIRE(StringUtils::trim("   world   ") == "world");
    REQUIRE(StringUtils::trim("\thello\t") == "hello");
    REQUIRE(StringUtils::trim("\nhello\n") == "hello");
    REQUIRE(StringUtils::trim("\r\n hello \r\n") == "hello");
}

TEST_CASE("StringUtils::trim - Mixed whitespace", "[StringUtils][Trim]") {
    REQUIRE(StringUtils::trim(" \t\nhello\t\n ") == "hello");
    REQUIRE(StringUtils::trim("\t \n \r world \r \n \t") == "world");
}

//============================================
// Trim Tests - Edge Cases
//============================================
TEST_CASE("StringUtils::trim - Empty string", "[StringUtils][Trim][EdgeCase]") {
    REQUIRE(StringUtils::trim("") == "");
}

TEST_CASE("StringUtils::trim - Only whitespace", "[StringUtils][Trim][EdgeCase]") {
    REQUIRE(StringUtils::trim(" ") == "");
    REQUIRE(StringUtils::trim("  ") == "");
    REQUIRE(StringUtils::trim("\t") == "");
    REQUIRE(StringUtils::trim("\n") == "");
    REQUIRE(StringUtils::trim("\r\n") == "");
    REQUIRE(StringUtils::trim(" \t\n\r ") == "");
}

TEST_CASE("StringUtils::trim - Single character", "[StringUtils][Trim][EdgeCase]") {
    REQUIRE(StringUtils::trim("a") == "a");
    REQUIRE(StringUtils::trim(" a") == "a");
    REQUIRE(StringUtils::trim("a ") == "a");
    REQUIRE(StringUtils::trim(" a ") == "a");
}

TEST_CASE("StringUtils::trim - Whitespace in middle preserved", "[StringUtils][Trim]") {
    REQUIRE(StringUtils::trim("hello world") == "hello world");
    REQUIRE(StringUtils::trim("  hello world  ") == "hello world");
    REQUIRE(StringUtils::trim("hello   world") == "hello   world");
    REQUIRE(StringUtils::trim("  hello   world  ") == "hello   world");
}

//============================================
// Trim Tests - Special Characters
//============================================
TEST_CASE("StringUtils::trim - Tab characters", "[StringUtils][Trim][Special]") {
    REQUIRE(StringUtils::trim("\t\thello\t\t") == "hello");
    REQUIRE(StringUtils::trim("hello\tworld") == "hello\tworld"); // Middle tab preserved
}

TEST_CASE("StringUtils::trim - Newline characters", "[StringUtils][Trim][Special]") {
    REQUIRE(StringUtils::trim("\n\nhello\n\n") == "hello");
    REQUIRE(StringUtils::trim("hello\nworld") == "hello\nworld"); // Middle newline preserved
}

TEST_CASE("StringUtils::trim - Carriage return", "[StringUtils][Trim][Special]") {
    REQUIRE(StringUtils::trim("\r\rhello\r\r") == "hello");
    REQUIRE(StringUtils::trim("\r\n\r\nhello\r\n\r\n") == "hello");
}

TEST_CASE("StringUtils::trim - Form feed and vertical tab", "[StringUtils][Trim][Special]") {
    REQUIRE(StringUtils::trim("\f\vhello\v\f") == "hello");
}

//============================================
// Trim Tests - Long Strings
//============================================
TEST_CASE("StringUtils::trim - Long string no whitespace", "[StringUtils][Trim]") {
    std::string longStr(1000, 'a');
    REQUIRE(StringUtils::trim(longStr) == longStr);
}

TEST_CASE("StringUtils::trim - Long string with whitespace", "[StringUtils][Trim]") {
    std::string content(1000, 'a');
    std::string withWhitespace = "   " + content + "   ";
    REQUIRE(StringUtils::trim(withWhitespace) == content);
}

TEST_CASE("StringUtils::trim - Long whitespace only", "[StringUtils][Trim]") {
    std::string longWhitespace(1000, ' ');
    REQUIRE(StringUtils::trim(longWhitespace) == "");
}

//============================================
// Trim Tests - Unicode (if supported)
//============================================
TEST_CASE("StringUtils::trim - ASCII special characters preserved", "[StringUtils][Trim]") {
    REQUIRE(StringUtils::trim("  !@#$%^&*()  ") == "!@#$%^&*()");
    REQUIRE(StringUtils::trim("  []{}<>  ") == "[]{}<>");
}

TEST_CASE("StringUtils::trim - Numeric strings", "[StringUtils][Trim]") {
    REQUIRE(StringUtils::trim("  123  ") == "123");
    REQUIRE(StringUtils::trim("  3.14159  ") == "3.14159");
    REQUIRE(StringUtils::trim("  -42  ") == "-42");
}

//============================================
// Trim Tests - Practical Use Cases
//============================================
TEST_CASE("StringUtils::trim - User input simulation", "[StringUtils][Trim][Practical]") {
    // Simulating various user inputs
    REQUIRE(StringUtils::trim("  John Doe  ") == "John Doe");
    REQUIRE(StringUtils::trim("john@example.com ") == "john@example.com");
    REQUIRE(StringUtils::trim(" password123") == "password123");
}

TEST_CASE("StringUtils::trim - File content simulation", "[StringUtils][Trim][Practical]") {
    // Simulating lines from a file
    REQUIRE(StringUtils::trim("key=value\n") == "key=value");
    REQUIRE(StringUtils::trim("  # comment  \n") == "# comment");
    REQUIRE(StringUtils::trim("\t\tindented content\r\n") == "indented content");
}

TEST_CASE("StringUtils::trim - Path-like strings", "[StringUtils][Trim][Practical]") {
    REQUIRE(StringUtils::trim("  /usr/local/bin  ") == "/usr/local/bin");
    REQUIRE(StringUtils::trim("  C:\\Program Files\\  ") == "C:\\Program Files\\");
}

//============================================
// Trim Tests - Multiple calls
//============================================
TEST_CASE("StringUtils::trim - Idempotent", "[StringUtils][Trim]") {
    // Trimming an already trimmed string should return same result
    std::string original = "  hello world  ";
    std::string trimmed1 = StringUtils::trim(original);
    std::string trimmed2 = StringUtils::trim(trimmed1);
    std::string trimmed3 = StringUtils::trim(trimmed2);
    
    REQUIRE(trimmed1 == "hello world");
    REQUIRE(trimmed2 == "hello world");
    REQUIRE(trimmed3 == "hello world");
}

TEST_CASE("StringUtils::trim - Consecutive trims", "[StringUtils][Trim]") {
    std::vector<std::string> inputs = {
        "  test1  ",
        "\ttest2\t",
        "\ntest3\n",
        "  \t\n test4 \n\t  "
    };
    
    for (const auto& input : inputs) {
        auto trimmed = StringUtils::trim(input);
        REQUIRE(StringUtils::trim(trimmed) == trimmed);
    }
}

//============================================
// Trim Tests - Const correctness
//============================================
TEST_CASE("StringUtils::trim - Original unchanged", "[StringUtils][Trim]") {
    const std::string original = "  hello  ";
    std::string result = StringUtils::trim(original);
    
    REQUIRE(original == "  hello  "); // Original should be unchanged
    REQUIRE(result == "hello");
}
