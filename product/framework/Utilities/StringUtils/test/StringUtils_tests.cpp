#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <ucf/Utilities/StringUtils/StringUtils.h>

#include <string>
#include <vector>
#include <limits>

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
//============================================
// ltrim Tests
//============================================
TEST_CASE("StringUtils::ltrim - Basic", "[StringUtils][ltrim]") {
    REQUIRE(StringUtils::ltrim("  hello") == "hello");
    REQUIRE(StringUtils::ltrim("\t\nhello") == "hello");
    REQUIRE(StringUtils::ltrim("hello  ") == "hello  ");  // 右侧保留
    REQUIRE(StringUtils::ltrim("  hello  ") == "hello  ");
}

TEST_CASE("StringUtils::ltrim - Edge cases", "[StringUtils][ltrim][EdgeCase]") {
    REQUIRE(StringUtils::ltrim("") == "");
    REQUIRE(StringUtils::ltrim("   ") == "");
    REQUIRE(StringUtils::ltrim("a") == "a");
    REQUIRE(StringUtils::ltrim("hello") == "hello");
}

//============================================
// rtrim Tests
//============================================
TEST_CASE("StringUtils::rtrim - Basic", "[StringUtils][rtrim]") {
    REQUIRE(StringUtils::rtrim("hello  ") == "hello");
    REQUIRE(StringUtils::rtrim("hello\t\n") == "hello");
    REQUIRE(StringUtils::rtrim("  hello") == "  hello");  // 左侧保留
    REQUIRE(StringUtils::rtrim("  hello  ") == "  hello");
}

TEST_CASE("StringUtils::rtrim - Edge cases", "[StringUtils][rtrim][EdgeCase]") {
    REQUIRE(StringUtils::rtrim("") == "");
    REQUIRE(StringUtils::rtrim("   ") == "");
    REQUIRE(StringUtils::rtrim("a") == "a");
    REQUIRE(StringUtils::rtrim("hello") == "hello");
}

//============================================
// toUpper Tests
//============================================
TEST_CASE("StringUtils::toUpper - Basic", "[StringUtils][toUpper]") {
    REQUIRE(StringUtils::toUpper("hello") == "HELLO");
    REQUIRE(StringUtils::toUpper("Hello World") == "HELLO WORLD");
    REQUIRE(StringUtils::toUpper("abc123") == "ABC123");
}

TEST_CASE("StringUtils::toUpper - Edge cases", "[StringUtils][toUpper][EdgeCase]") {
    REQUIRE(StringUtils::toUpper("") == "");
    REQUIRE(StringUtils::toUpper("ALREADY UPPER") == "ALREADY UPPER");
    REQUIRE(StringUtils::toUpper("123!@#") == "123!@#");
    REQUIRE(StringUtils::toUpper("a") == "A");
    REQUIRE(StringUtils::toUpper("Z") == "Z");
}

TEST_CASE("StringUtils::toUpper - Mixed", "[StringUtils][toUpper]") {
    REQUIRE(StringUtils::toUpper("HeLLo WoRLd") == "HELLO WORLD");
    REQUIRE(StringUtils::toUpper("Test123Test") == "TEST123TEST");
}

//============================================
// toLower Tests
//============================================
TEST_CASE("StringUtils::toLower - Basic", "[StringUtils][toLower]") {
    REQUIRE(StringUtils::toLower("HELLO") == "hello");
    REQUIRE(StringUtils::toLower("Hello World") == "hello world");
    REQUIRE(StringUtils::toLower("ABC123") == "abc123");
}

TEST_CASE("StringUtils::toLower - Edge cases", "[StringUtils][toLower][EdgeCase]") {
    REQUIRE(StringUtils::toLower("") == "");
    REQUIRE(StringUtils::toLower("already lower") == "already lower");
    REQUIRE(StringUtils::toLower("123!@#") == "123!@#");
    REQUIRE(StringUtils::toLower("A") == "a");
    REQUIRE(StringUtils::toLower("z") == "z");
}

//============================================
// split (char delimiter) Tests
//============================================
TEST_CASE("StringUtils::split(char) - Basic", "[StringUtils][split]") {
    auto parts = StringUtils::split("a,b,c", ',');
    REQUIRE(parts.size() == 3);
    REQUIRE(parts[0] == "a");
    REQUIRE(parts[1] == "b");
    REQUIRE(parts[2] == "c");
}

TEST_CASE("StringUtils::split(char) - Edge cases", "[StringUtils][split][EdgeCase]") {
    // 空字符串 - istringstream getline 返回空 vector
    auto empty = StringUtils::split("", ',');
    REQUIRE(empty.empty());
    
    // 无分隔符
    auto noDelim = StringUtils::split("hello", ',');
    REQUIRE(noDelim.size() == 1);
    REQUIRE(noDelim[0] == "hello");
    
    // 只有分隔符 - istringstream getline 返回1个空字符串
    auto onlyDelim = StringUtils::split(",", ',');
    REQUIRE(onlyDelim.size() == 1);  // getline behavior
    REQUIRE(onlyDelim[0] == "");
    
    // 连续分隔符
    auto consecutive = StringUtils::split("a,,b", ',');
    REQUIRE(consecutive.size() == 3);
    REQUIRE(consecutive[0] == "a");
    REQUIRE(consecutive[1] == "");
    REQUIRE(consecutive[2] == "b");
    
    // 开头有分隔符
    auto startDelim = StringUtils::split(",a,b", ',');
    REQUIRE(startDelim.size() == 3);
    REQUIRE(startDelim[0] == "");
    REQUIRE(startDelim[1] == "a");
    REQUIRE(startDelim[2] == "b");
    
    // 结尾有分隔符 - istringstream 不保留最后的空元素
    auto endDelim = StringUtils::split("a,b,", ',');
    REQUIRE(endDelim.size() == 2);  // getline 不产生最后的空元素
    REQUIRE(endDelim[0] == "a");
    REQUIRE(endDelim[1] == "b");
}

//============================================
// split (string delimiter) Tests
//============================================
TEST_CASE("StringUtils::split(string) - Basic", "[StringUtils][split]") {
    auto parts = StringUtils::split("a::b::c", "::");
    REQUIRE(parts.size() == 3);
    REQUIRE(parts[0] == "a");
    REQUIRE(parts[1] == "b");
    REQUIRE(parts[2] == "c");
}

TEST_CASE("StringUtils::split(string) - Edge cases", "[StringUtils][split][EdgeCase]") {
    // 空分隔符
    auto emptyDelim = StringUtils::split("hello", "");
    REQUIRE(emptyDelim.size() == 1);
    REQUIRE(emptyDelim[0] == "hello");
    
    // 分隔符不存在
    auto noMatch = StringUtils::split("hello", "xyz");
    REQUIRE(noMatch.size() == 1);
    REQUIRE(noMatch[0] == "hello");
    
    // 空字符串
    auto empty = StringUtils::split("", "::");
    REQUIRE(empty.size() == 1);
    REQUIRE(empty[0] == "");
    
    // 长分隔符
    auto longDelim = StringUtils::split("a<-->b<-->c", "<-->");
    REQUIRE(longDelim.size() == 3);
}

//============================================
// join Tests
//============================================
TEST_CASE("StringUtils::join - Basic", "[StringUtils][join]") {
    std::vector<std::string> parts = {"a", "b", "c"};
    REQUIRE(StringUtils::join(parts, ",") == "a,b,c");
    REQUIRE(StringUtils::join(parts, "::") == "a::b::c");
    REQUIRE(StringUtils::join(parts, "") == "abc");
}

TEST_CASE("StringUtils::join - Edge cases", "[StringUtils][join][EdgeCase]") {
    // 空数组
    std::vector<std::string> empty;
    REQUIRE(StringUtils::join(empty, ",") == "");
    
    // 单元素
    std::vector<std::string> single = {"hello"};
    REQUIRE(StringUtils::join(single, ",") == "hello");
    
    // 包含空字符串
    std::vector<std::string> withEmpty = {"a", "", "c"};
    REQUIRE(StringUtils::join(withEmpty, ",") == "a,,c");
}

TEST_CASE("StringUtils::split and join - Roundtrip", "[StringUtils][split][join]") {
    std::string original = "a,b,c,d,e";
    auto parts = StringUtils::split(original, ',');
    auto rejoined = StringUtils::join(parts, ",");
    REQUIRE(rejoined == original);
}

//============================================
// startsWith Tests
//============================================
TEST_CASE("StringUtils::startsWith - Basic", "[StringUtils][startsWith]") {
    REQUIRE(StringUtils::startsWith("hello world", "hello"));
    REQUIRE(StringUtils::startsWith("hello", "hello"));
    REQUIRE_FALSE(StringUtils::startsWith("hello world", "world"));
    REQUIRE_FALSE(StringUtils::startsWith("hello", "Hello"));  // 大小写敏感
}

TEST_CASE("StringUtils::startsWith - Edge cases", "[StringUtils][startsWith][EdgeCase]") {
    REQUIRE(StringUtils::startsWith("hello", ""));  // 空前缀
    REQUIRE(StringUtils::startsWith("", ""));  // 两者都空
    REQUIRE_FALSE(StringUtils::startsWith("", "a"));  // 空字符串
    REQUIRE_FALSE(StringUtils::startsWith("hi", "hello"));  // 前缀比字符串长
    REQUIRE(StringUtils::startsWith("a", "a"));
}

//============================================
// endsWith Tests
//============================================
TEST_CASE("StringUtils::endsWith - Basic", "[StringUtils][endsWith]") {
    REQUIRE(StringUtils::endsWith("hello world", "world"));
    REQUIRE(StringUtils::endsWith("hello", "hello"));
    REQUIRE_FALSE(StringUtils::endsWith("hello world", "hello"));
    REQUIRE_FALSE(StringUtils::endsWith("hello", "HELLO"));  // 大小写敏感
}

TEST_CASE("StringUtils::endsWith - Edge cases", "[StringUtils][endsWith][EdgeCase]") {
    REQUIRE(StringUtils::endsWith("hello", ""));  // 空后缀
    REQUIRE(StringUtils::endsWith("", ""));  // 两者都空
    REQUIRE_FALSE(StringUtils::endsWith("", "a"));  // 空字符串
    REQUIRE_FALSE(StringUtils::endsWith("hi", "hello"));  // 后缀比字符串长
    REQUIRE(StringUtils::endsWith("a", "a"));
}

//============================================
// contains Tests
//============================================
TEST_CASE("StringUtils::contains - Basic", "[StringUtils][contains]") {
    REQUIRE(StringUtils::contains("hello world", "world"));
    REQUIRE(StringUtils::contains("hello world", "hello"));
    REQUIRE(StringUtils::contains("hello world", "lo wo"));
    REQUIRE(StringUtils::contains("hello", "hello"));
    REQUIRE_FALSE(StringUtils::contains("hello", "xyz"));
}

TEST_CASE("StringUtils::contains - Edge cases", "[StringUtils][contains][EdgeCase]") {
    REQUIRE(StringUtils::contains("hello", ""));  // 空子串
    REQUIRE(StringUtils::contains("", ""));  // 两者都空
    REQUIRE_FALSE(StringUtils::contains("", "a"));  // 空字符串
    REQUIRE_FALSE(StringUtils::contains("hi", "hello"));  // 子串比字符串长
    REQUIRE(StringUtils::contains("aaa", "aa"));  // 重叠匹配
}

//============================================
// replace Tests
//============================================
TEST_CASE("StringUtils::replace - Basic", "[StringUtils][replace]") {
    REQUIRE(StringUtils::replace("hello world", "world", "there") == "hello there");
    REQUIRE(StringUtils::replace("hello hello", "hello", "hi") == "hi hello");  // 只替换第一个
    REQUIRE(StringUtils::replace("hello", "xyz", "abc") == "hello");  // 不匹配
}

TEST_CASE("StringUtils::replace - Edge cases", "[StringUtils][replace][EdgeCase]") {
    REQUIRE(StringUtils::replace("hello", "", "x") == "hello");  // 空from
    REQUIRE(StringUtils::replace("hello", "l", "") == "helo");  // 替换为空
    REQUIRE(StringUtils::replace("", "a", "b") == "");  // 空字符串
    REQUIRE(StringUtils::replace("aaa", "a", "bb") == "bbaa");  // 替换后变长
    REQUIRE(StringUtils::replace("hello", "hello", "") == "");  // 全部删除
}

//============================================
// replaceAll Tests
//============================================
TEST_CASE("StringUtils::replaceAll - Basic", "[StringUtils][replaceAll]") {
    REQUIRE(StringUtils::replaceAll("hello world", "o", "0") == "hell0 w0rld");
    REQUIRE(StringUtils::replaceAll("hello hello", "hello", "hi") == "hi hi");
    REQUIRE(StringUtils::replaceAll("aaa", "a", "b") == "bbb");
}

TEST_CASE("StringUtils::replaceAll - Edge cases", "[StringUtils][replaceAll][EdgeCase]") {
    REQUIRE(StringUtils::replaceAll("hello", "", "x") == "hello");  // 空from
    REQUIRE(StringUtils::replaceAll("hello", "l", "") == "heo");  // 替换为空
    REQUIRE(StringUtils::replaceAll("", "a", "b") == "");  // 空字符串
    REQUIRE(StringUtils::replaceAll("hello", "xyz", "abc") == "hello");  // 不匹配
    
    // 替换后形成新的匹配项（不应重复替换）
    REQUIRE(StringUtils::replaceAll("aa", "a", "aa") == "aaaa");
}

TEST_CASE("StringUtils::replaceAll - Complex patterns", "[StringUtils][replaceAll]") {
    REQUIRE(StringUtils::replaceAll("ababab", "ab", "c") == "ccc");
    REQUIRE(StringUtils::replaceAll("a::b::c", "::", "/") == "a/b/c");
}

//============================================
// toInt Tests
//============================================
TEST_CASE("StringUtils::toInt - Basic", "[StringUtils][toInt]") {
    REQUIRE(StringUtils::toInt("123").value() == 123);
    REQUIRE(StringUtils::toInt("-456").value() == -456);
    REQUIRE(StringUtils::toInt("0").value() == 0);
}

TEST_CASE("StringUtils::toInt - Edge cases", "[StringUtils][toInt][EdgeCase]") {
    REQUIRE_FALSE(StringUtils::toInt("").has_value());
    REQUIRE_FALSE(StringUtils::toInt("abc").has_value());
    REQUIRE_FALSE(StringUtils::toInt("12.34").has_value());  // 不是整数
    REQUIRE_FALSE(StringUtils::toInt("12abc").has_value());  // 混合
    REQUIRE_FALSE(StringUtils::toInt("  123").has_value());  // 前导空格
    REQUIRE_FALSE(StringUtils::toInt("123  ").has_value());  // 尾部空格
}

TEST_CASE("StringUtils::toInt - Boundary values", "[StringUtils][toInt][Boundary]") {
    REQUIRE(StringUtils::toInt("2147483647").value() == 2147483647);  // INT_MAX
    REQUIRE(StringUtils::toInt("-2147483648").value() == -2147483648);  // INT_MIN
    REQUIRE_FALSE(StringUtils::toInt("2147483648").has_value());  // 溢出
    REQUIRE_FALSE(StringUtils::toInt("-2147483649").has_value());  // 下溢
}

//============================================
// toInt64 Tests
//============================================
TEST_CASE("StringUtils::toInt64 - Basic", "[StringUtils][toInt64]") {
    REQUIRE(StringUtils::toInt64("123").value() == 123);
    REQUIRE(StringUtils::toInt64("-456").value() == -456);
    REQUIRE(StringUtils::toInt64("9223372036854775807").value() == 9223372036854775807LL);  // INT64_MAX
}

TEST_CASE("StringUtils::toInt64 - Edge cases", "[StringUtils][toInt64][EdgeCase]") {
    REQUIRE_FALSE(StringUtils::toInt64("").has_value());
    REQUIRE_FALSE(StringUtils::toInt64("abc").has_value());
    REQUIRE_FALSE(StringUtils::toInt64("9223372036854775808").has_value());  // 溢出
}

//============================================
// toDouble Tests
//============================================
TEST_CASE("StringUtils::toDouble - Basic", "[StringUtils][toDouble]") {
    REQUIRE(StringUtils::toDouble("3.14").value() == Catch::Approx(3.14));
    REQUIRE(StringUtils::toDouble("-2.5").value() == Catch::Approx(-2.5));
    REQUIRE(StringUtils::toDouble("0.0").value() == Catch::Approx(0.0));
    REQUIRE(StringUtils::toDouble("123").value() == Catch::Approx(123.0));
}

TEST_CASE("StringUtils::toDouble - Scientific notation", "[StringUtils][toDouble]") {
    REQUIRE(StringUtils::toDouble("1e10").value() == Catch::Approx(1e10));
    REQUIRE(StringUtils::toDouble("1.5e-3").value() == Catch::Approx(0.0015));
    REQUIRE(StringUtils::toDouble("-2.5E+2").value() == Catch::Approx(-250.0));
}

TEST_CASE("StringUtils::toDouble - Edge cases", "[StringUtils][toDouble][EdgeCase]") {
    REQUIRE_FALSE(StringUtils::toDouble("").has_value());
    REQUIRE_FALSE(StringUtils::toDouble("abc").has_value());
    REQUIRE_FALSE(StringUtils::toDouble("12.34.56").has_value());
    REQUIRE_FALSE(StringUtils::toDouble("12abc").has_value());
}

//============================================
// repeat Tests
//============================================
TEST_CASE("StringUtils::repeat - Basic", "[StringUtils][repeat]") {
    REQUIRE(StringUtils::repeat("ab", 3) == "ababab");
    REQUIRE(StringUtils::repeat("x", 5) == "xxxxx");
    REQUIRE(StringUtils::repeat("hello", 1) == "hello");
}

TEST_CASE("StringUtils::repeat - Edge cases", "[StringUtils][repeat][EdgeCase]") {
    REQUIRE(StringUtils::repeat("hello", 0) == "");
    REQUIRE(StringUtils::repeat("", 5) == "");
    REQUIRE(StringUtils::repeat("", 0) == "");
    REQUIRE(StringUtils::repeat("a", 1) == "a");
}

//============================================
// padLeft Tests
//============================================
TEST_CASE("StringUtils::padLeft - Basic", "[StringUtils][padLeft]") {
    REQUIRE(StringUtils::padLeft("42", 5) == "   42");
    REQUIRE(StringUtils::padLeft("42", 5, '0') == "00042");
    REQUIRE(StringUtils::padLeft("hello", 10, '-') == "-----hello");
}

TEST_CASE("StringUtils::padLeft - Edge cases", "[StringUtils][padLeft][EdgeCase]") {
    REQUIRE(StringUtils::padLeft("hello", 5) == "hello");  // 宽度等于长度
    REQUIRE(StringUtils::padLeft("hello", 3) == "hello");  // 宽度小于长度
    REQUIRE(StringUtils::padLeft("", 3) == "   ");
    REQUIRE(StringUtils::padLeft("", 0) == "");
}

//============================================
// padRight Tests
//============================================
TEST_CASE("StringUtils::padRight - Basic", "[StringUtils][padRight]") {
    REQUIRE(StringUtils::padRight("42", 5) == "42   ");
    REQUIRE(StringUtils::padRight("42", 5, '0') == "42000");
    REQUIRE(StringUtils::padRight("hello", 10, '-') == "hello-----");
}

TEST_CASE("StringUtils::padRight - Edge cases", "[StringUtils][padRight][EdgeCase]") {
    REQUIRE(StringUtils::padRight("hello", 5) == "hello");  // 宽度等于长度
    REQUIRE(StringUtils::padRight("hello", 3) == "hello");  // 宽度小于长度
    REQUIRE(StringUtils::padRight("", 3) == "   ");
    REQUIRE(StringUtils::padRight("", 0) == "");
}