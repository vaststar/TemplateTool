#include <catch2/catch_test_macros.hpp>
#include <ucf/Utilities/StringUtils/UnicodeString.h>
#include <ucf/Utilities/StringUtils/StringUtils.h>

#include <vector>


// Helper macro to convert u8 literals for C++20 compatibility
#define U8STR(x) reinterpret_cast<const char*>(u8##x)

TEST_CASE("UnicodeString - default construction", "[UnicodeString]")
{
    ucf::utilities::UnicodeString s;
    REQUIRE(s.empty());
    REQUIRE(s.length() == 0);
    REQUIRE(s.byteLength() == 0);
    REQUIRE(s.toStdString().empty());
}

TEST_CASE("UnicodeString - construction from ASCII", "[UnicodeString]")
{
    ucf::utilities::UnicodeString s = "hello";
    REQUIRE(s.length() == 5);
    REQUIRE(s.byteLength() == 5);
    REQUIRE(s.isAscii());
    REQUIRE(s.isValid());
    REQUIRE(s.toStdString() == "hello");
}

TEST_CASE("UnicodeString - construction from UTF-8", "[UnicodeString]")
{
    ucf::utilities::UnicodeString s = U8STR("你好");
    REQUIRE(s.length() == 2);
    REQUIRE(s.byteLength() == 6);  // 每个中文字符 3 字节
    REQUIRE_FALSE(s.isAscii());
    REQUIRE(s.isValid());
}

TEST_CASE("UnicodeString - construction from wide string", "[UnicodeString]")
{
    ucf::utilities::UnicodeString s = ucf::utilities::UnicodeString::fromWide(L"世界");
    REQUIRE(s.length() == 2);
    REQUIRE_FALSE(s.isAscii());
}

TEST_CASE("UnicodeString - construction from codepoint", "[UnicodeString]")
{
    ucf::utilities::UnicodeString s(U'中');
    REQUIRE(s.length() == 1);
    REQUIRE(s.byteLength() == 3);
    REQUIRE(s[0] == U'中');
}

TEST_CASE("UnicodeString - copy and move", "[UnicodeString]")
{
    ucf::utilities::UnicodeString original = U8STR("测试");
    
    SECTION("copy constructor") {
        ucf::utilities::UnicodeString copy(original);
        REQUIRE(copy == original);
        REQUIRE(copy.length() == 2);
    }
    
    SECTION("copy assignment") {
        ucf::utilities::UnicodeString copy;
        copy = original;
        REQUIRE(copy == original);
    }
    
    SECTION("move constructor") {
        ucf::utilities::UnicodeString temp = original;
        ucf::utilities::UnicodeString moved(std::move(temp));
        REQUIRE(moved == original);
    }
    
    SECTION("move assignment") {
        ucf::utilities::UnicodeString temp = original;
        ucf::utilities::UnicodeString moved;
        moved = std::move(temp);
        REQUIRE(moved == original);
    }
}

TEST_CASE("UnicodeString - conversion to std::string", "[UnicodeString]")
{
    ucf::utilities::UnicodeString s = U8STR("测试Test");
    
    std::string utf8 = s.toStdString();
    REQUIRE(utf8 == U8STR("测试Test"));
    
    // 隐式转换
    std::string implicit = s;
    REQUIRE(implicit == U8STR("测试Test"));
    
    // string_view
    std::string_view view = s.toStringView();
    REQUIRE(view == std::string_view(U8STR("测试Test")));
}

TEST_CASE("UnicodeString - conversion to wide string", "[UnicodeString]")
{
    ucf::utilities::UnicodeString s = U8STR("测试Test");
    std::wstring wide = s.toWide();
    REQUIRE(wide == L"测试Test");
}

TEST_CASE("UnicodeString - fromUtf16 and toUtf16", "[UnicodeString]")
{
    std::u16string utf16 = u"Hello世界";
    ucf::utilities::UnicodeString s = ucf::utilities::UnicodeString::fromUtf16(utf16);
    
    REQUIRE(s.length() == 7);
    REQUIRE(s.toUtf16() == utf16);
}

TEST_CASE("UnicodeString - fromUtf32 and toUtf32", "[UnicodeString]")
{
    std::u32string utf32 = U"Hello世界";
    ucf::utilities::UnicodeString s = ucf::utilities::UnicodeString::fromUtf32(utf32);
    
    REQUIRE(s.length() == 7);
    REQUIRE(s.toUtf32() == utf32);
}

TEST_CASE("UnicodeString - character access by index", "[UnicodeString]")
{
    ucf::utilities::UnicodeString s = U8STR("A中B");
    
    REQUIRE(s[0] == U'A');
    REQUIRE(s[1] == U'中');
    REQUIRE(s[2] == U'B');
    
    REQUIRE(s.at(0) == U'A');
    REQUIRE(s.at(1) == U'中');
    REQUIRE(s.at(2) == U'B');
    
    REQUIRE_THROWS_AS(s.at(10), std::out_of_range);
}

TEST_CASE("UnicodeString - front and back", "[UnicodeString]")
{
    ucf::utilities::UnicodeString s = U8STR("A中B");
    
    REQUIRE(s.front() == U'A');
    REQUIRE(s.back() == U'B');
    
    ucf::utilities::UnicodeString empty;
    REQUIRE_THROWS_AS(empty.front(), std::out_of_range);
    REQUIRE_THROWS_AS(empty.back(), std::out_of_range);
}

TEST_CASE("UnicodeString - iteration", "[UnicodeString]")
{
    ucf::utilities::UnicodeString s = U8STR("你好");
    std::vector<char32_t> chars;
    
    for (char32_t ch : s) {
        chars.push_back(ch);
    }
    
    REQUIRE(chars.size() == 2);
    REQUIRE(chars[0] == U'你');
    REQUIRE(chars[1] == U'好');
}

TEST_CASE("UnicodeString - iteration with mixed content", "[UnicodeString]")
{
    ucf::utilities::UnicodeString s = U8STR("a中b");
    std::vector<char32_t> chars;
    
    for (auto it = s.begin(); it != s.end(); ++it) {
        chars.push_back(*it);
    }
    
    REQUIRE(chars.size() == 3);
    REQUIRE(chars[0] == U'a');
    REQUIRE(chars[1] == U'中');
    REQUIRE(chars[2] == U'b');
}

TEST_CASE("UnicodeString - concatenation", "[UnicodeString]")
{
    ucf::utilities::UnicodeString a = "Hello";
    ucf::utilities::UnicodeString b = ucf::utilities::UnicodeString::fromWide(L"世界");
    
    SECTION("ucf::utilities::UnicodeString + ucf::utilities::UnicodeString") {
        ucf::utilities::UnicodeString c = a + " " + b;
        REQUIRE(c.length() == 8);
    }
    
    SECTION("ucf::utilities::UnicodeString + std::string") {
        ucf::utilities::UnicodeString c = a + std::string(" World");
        REQUIRE(c == "Hello World");
    }
    
    SECTION("ucf::utilities::UnicodeString + const char*") {
        ucf::utilities::UnicodeString c = a + " World";
        REQUIRE(c == "Hello World");
    }
    
    SECTION("ucf::utilities::UnicodeString + char32_t") {
        ucf::utilities::UnicodeString c = a + U'!';
        REQUIRE(c == "Hello!");
    }
    
    SECTION("const char* + ucf::utilities::UnicodeString") {
        ucf::utilities::UnicodeString c = "Say: " + b;
        REQUIRE(c.length() == 7);
    }
}

TEST_CASE("UnicodeString - operator+=", "[UnicodeString]")
{
    ucf::utilities::UnicodeString s = "Hello";
    
    s += " ";
    s += ucf::utilities::UnicodeString::fromWide(L"世界");
    s += U'!';
    
    REQUIRE(s.length() == 9);
}

TEST_CASE("UnicodeString - comparison operators", "[UnicodeString]")
{
    ucf::utilities::UnicodeString a = "abc";
    ucf::utilities::UnicodeString b = "abc";
    ucf::utilities::UnicodeString c = "abd";
    ucf::utilities::UnicodeString d = "ab";
    
    REQUIRE(a == b);
    REQUIRE(a != c);
    REQUIRE(a < c);
    REQUIRE(a <= b);
    REQUIRE(c > a);
    REQUIRE(c >= a);
    REQUIRE(a > d);
}

TEST_CASE("UnicodeString - comparison with std::string", "[UnicodeString]")
{
    ucf::utilities::UnicodeString s = "hello";
    
    REQUIRE(s == std::string("hello"));
    REQUIRE(s == "hello");
    REQUIRE(std::string("hello") == s);
    REQUIRE("hello" == s);
    
    REQUIRE_FALSE(s == "world");
}

TEST_CASE("UnicodeString - substr", "[UnicodeString]")
{
    ucf::utilities::UnicodeString s = ucf::utilities::UnicodeString::fromWide(L"你好世界");
    
    ucf::utilities::UnicodeString sub1 = s.substr(0, 2);
    REQUIRE(sub1 == ucf::utilities::UnicodeString::fromWide(L"你好"));
    
    ucf::utilities::UnicodeString sub2 = s.substr(2);
    REQUIRE(sub2 == ucf::utilities::UnicodeString::fromWide(L"世界"));
    
    ucf::utilities::UnicodeString sub3 = s.substr(1, 2);
    REQUIRE(sub3 == ucf::utilities::UnicodeString::fromWide(L"好世"));
    
    ucf::utilities::UnicodeString sub4 = s.substr(0, 100);  // count 超过长度
    REQUIRE(sub4.length() == 4);
    
    REQUIRE(s.substr(10).empty());  // pos 超过长度
}

TEST_CASE("UnicodeString - validation", "[UnicodeString]")
{
    SECTION("valid UTF-8") {
        ucf::utilities::UnicodeString s = ucf::utilities::UnicodeString::fromWide(L"Valid UTF-8 字符串");
        REQUIRE(s.isValid());
    }
    
    SECTION("ASCII check") {
        ucf::utilities::UnicodeString ascii = "pure ascii";
        ucf::utilities::UnicodeString unicode = ucf::utilities::UnicodeString::fromWide(L"包含中文");
        
        REQUIRE(ascii.isAscii());
        REQUIRE_FALSE(unicode.isAscii());
    }
    
    SECTION("static validation") {
        REQUIRE(ucf::utilities::UnicodeString::isValidUtf8("ascii"));
    }
}

TEST_CASE("UnicodeString - clear and reserve", "[UnicodeString]")
{
    ucf::utilities::UnicodeString s = "Hello World";
    
    REQUIRE_FALSE(s.empty());
    
    s.clear();
    REQUIRE(s.empty());
    REQUIRE(s.length() == 0);
    
    s.reserve(100);
    REQUIRE(s.capacity() >= 100);
}

TEST_CASE("UnicodeString - c_str and data", "[UnicodeString]")
{
    ucf::utilities::UnicodeString s = "test";
    
    REQUIRE(std::string(s.c_str()) == "test");
    REQUIRE(std::string(s.data(), s.byteLength()) == "test");
}

TEST_CASE("UnicodeString - literal suffix", "[UnicodeString]")
{
    using ucf::utilities::literals::operator""_us;
    
    ucf::utilities::UnicodeString s = "hello"_us;
    REQUIRE(s == "hello");
    REQUIRE(s.length() == 5);
}

TEST_CASE("UnicodeString - null pointer handling", "[UnicodeString]")
{
    ucf::utilities::UnicodeString s1(static_cast<const char*>(nullptr));
    REQUIRE(s1.empty());
    
    ucf::utilities::UnicodeString s2 = ucf::utilities::UnicodeString::fromWide(static_cast<const wchar_t*>(nullptr));
    REQUIRE(s2.empty());
    
    ucf::utilities::UnicodeString s3 = ucf::utilities::UnicodeString::fromUtf16(static_cast<const char16_t*>(nullptr));
    REQUIRE(s3.empty());
}

TEST_CASE("UnicodeString - emoji support", "[UnicodeString]")
{
    // 测试 4 字节 UTF-8 字符 (emoji) - 使用 fromWide 避免 u8 问题
    ucf::utilities::UnicodeString hello = "Hello ";
    ucf::utilities::UnicodeString emoji = ucf::utilities::UnicodeString::fromUtf32(U"\U0001F600");
    ucf::utilities::UnicodeString world = " World";
    ucf::utilities::UnicodeString s = hello + emoji + world;
    
    REQUIRE(s.isValid());
    // "Hello " = 6, emoji = 1, " World" = 6
    REQUIRE(s.length() == 13);
}

TEST_CASE("UnicodeString - surrogate pairs in UTF-16", "[UnicodeString]")
{
    // 使用需要 surrogate pair 的字符 (emoji 或 SMP 字符)
    std::u16string utf16 = u"A\U0001F600B";  // A, 😀, B
    ucf::utilities::UnicodeString s = ucf::utilities::UnicodeString::fromUtf16(utf16);
    
    REQUIRE(s.length() == 3);
    REQUIRE(s[0] == U'A');
    REQUIRE(s[1] == U'\U0001F600');
    REQUIRE(s[2] == U'B');
    
    // 往返转换
    REQUIRE(s.toUtf16() == utf16);
}

TEST_CASE("StringUtils with UnicodeString - trim", "[UnicodeString][StringUtils]")
{
    ucf::utilities::UnicodeString s = ucf::utilities::UnicodeString::fromWide(L"  你好  ");
    ucf::utilities::UnicodeString trimmed = ucf::utilities::StringUtils::trim(s);
    REQUIRE(trimmed == ucf::utilities::UnicodeString::fromWide(L"你好"));
}

TEST_CASE("StringUtils with UnicodeString - contains", "[UnicodeString][StringUtils]")
{
    ucf::utilities::UnicodeString s = ucf::utilities::UnicodeString::fromWide(L"Hello世界");
    REQUIRE(ucf::utilities::StringUtils::contains(s, ucf::utilities::UnicodeString::fromWide(L"世")));
    REQUIRE_FALSE(ucf::utilities::StringUtils::contains(s, ucf::utilities::UnicodeString::fromWide(L"中")));
}

TEST_CASE("StringUtils with UnicodeString - startsWith/endsWith", "[UnicodeString][StringUtils]")
{
    ucf::utilities::UnicodeString s = ucf::utilities::UnicodeString::fromWide(L"北京市朝阳区");
    REQUIRE(ucf::utilities::StringUtils::startsWith(s, ucf::utilities::UnicodeString::fromWide(L"北京")));
    REQUIRE(ucf::utilities::StringUtils::endsWith(s, ucf::utilities::UnicodeString::fromWide(L"区")));
}

//============================================
// 边界条件测试 - 空字符串
//============================================
TEST_CASE("UnicodeString - empty string operations", "[UnicodeString][EdgeCase]")
{
    ucf::utilities::UnicodeString empty;
    
    SECTION("length and size") {
        REQUIRE(empty.length() == 0);
        REQUIRE(empty.size() == 0);
        REQUIRE(empty.byteLength() == 0);
        REQUIRE(empty.byteSize() == 0);
    }
    
    SECTION("empty check") {
        REQUIRE(empty.empty());
    }
    
    SECTION("c_str and data") {
        REQUIRE(empty.c_str() != nullptr);
        REQUIRE(std::string(empty.c_str()) == "");
    }
    
    SECTION("conversions") {
        REQUIRE(empty.toStdString() == "");
        REQUIRE(empty.toWide() == L"");
        REQUIRE(empty.toUtf16() == u"");
        REQUIRE(empty.toUtf32() == U"");
    }
    
    SECTION("substr of empty") {
        REQUIRE(empty.substr(0).empty());
        REQUIRE(empty.substr(0, 0).empty());
        REQUIRE(empty.substr(5).empty());  // 超出范围
    }
    
    SECTION("validation") {
        REQUIRE(empty.isValid());
        REQUIRE(empty.isAscii());
    }
    
    SECTION("iteration") {
        int count = 0;
        for ([[maybe_unused]] char32_t ch : empty) {
            ++count;
        }
        REQUIRE(count == 0);
        REQUIRE(empty.begin() == empty.end());
    }
}

//============================================
// 边界条件测试 - 单字符
//============================================
TEST_CASE("UnicodeString - single character", "[UnicodeString][EdgeCase]")
{
    SECTION("ASCII") {
        ucf::utilities::UnicodeString s = "a";
        REQUIRE(s.length() == 1);
        REQUIRE(s.byteLength() == 1);
        REQUIRE(s[0] == U'a');
        REQUIRE(s.front() == U'a');
        REQUIRE(s.back() == U'a');
        REQUIRE(s.isAscii());
    }
    
    SECTION("2-byte UTF-8") {
        // Latin Extended: ñ = U+00F1 (2 bytes in UTF-8)
        ucf::utilities::UnicodeString s(U'\u00F1');
        REQUIRE(s.length() == 1);
        REQUIRE(s.byteLength() == 2);
        REQUIRE(s[0] == U'\u00F1');
        REQUIRE_FALSE(s.isAscii());
    }
    
    SECTION("3-byte UTF-8") {
        // CJK: 中 = U+4E2D (3 bytes in UTF-8)
        ucf::utilities::UnicodeString s(U'中');
        REQUIRE(s.length() == 1);
        REQUIRE(s.byteLength() == 3);
        REQUIRE(s[0] == U'中');
    }
    
    SECTION("4-byte UTF-8") {
        // Emoji: 😀 = U+1F600 (4 bytes in UTF-8)
        ucf::utilities::UnicodeString s(U'\U0001F600');
        REQUIRE(s.length() == 1);
        REQUIRE(s.byteLength() == 4);
        REQUIRE(s[0] == U'\U0001F600');
    }
}

//============================================
// 边界条件测试 - 索引访问
//============================================
TEST_CASE("UnicodeString - index boundary", "[UnicodeString][EdgeCase]")
{
    ucf::utilities::UnicodeString s = ucf::utilities::UnicodeString::fromWide(L"ABC");
    
    SECTION("valid indices") {
        REQUIRE(s.at(0) == U'A');
        REQUIRE(s.at(1) == U'B');
        REQUIRE(s.at(2) == U'C');
    }
    
    SECTION("at() throws on invalid index") {
        REQUIRE_THROWS_AS(s.at(3), std::out_of_range);
        REQUIRE_THROWS_AS(s.at(100), std::out_of_range);
        REQUIRE_THROWS_AS(s.at(static_cast<size_t>(-1)), std::out_of_range);
    }
    
    SECTION("operator[] behavior on boundary") {
        // operator[] 不做边界检查，越界是未定义行为
        // 这里只测试有效范围
        REQUIRE(s[0] == U'A');
        REQUIRE(s[2] == U'C');
    }
}

//============================================
// 边界条件测试 - substr
//============================================
TEST_CASE("UnicodeString - substr boundary", "[UnicodeString][EdgeCase]")
{
    ucf::utilities::UnicodeString s = ucf::utilities::UnicodeString::fromWide(L"你好世界");  // 4 chars
    
    SECTION("normal substr") {
        REQUIRE(s.substr(0, 2).length() == 2);
        REQUIRE(s.substr(2, 2).length() == 2);
    }
    
    SECTION("pos at end") {
        REQUIRE(s.substr(4).empty());
    }
    
    SECTION("pos beyond end") {
        REQUIRE(s.substr(10).empty());
        REQUIRE(s.substr(100).empty());
    }
    
    SECTION("count exceeds remaining") {
        REQUIRE(s.substr(2, 100).length() == 2);  // 只剩2个字符
    }
    
    SECTION("count = 0") {
        REQUIRE(s.substr(1, 0).empty());
    }
    
    SECTION("npos as count") {
        REQUIRE(s.substr(1, ucf::utilities::UnicodeString::npos).length() == 3);
    }
}

//============================================
// 边界条件测试 - 迭代器
//============================================
TEST_CASE("UnicodeString - iterator edge cases", "[UnicodeString][EdgeCase]")
{
    SECTION("empty string iterator") {
        ucf::utilities::UnicodeString empty;
        REQUIRE(empty.begin() == empty.end());
        REQUIRE(empty.cbegin() == empty.cend());
    }
    
    SECTION("single char iteration") {
        ucf::utilities::UnicodeString s(U'X');
        auto it = s.begin();
        REQUIRE(*it == U'X');
        ++it;
        REQUIRE(it == s.end());
    }
    
    SECTION("post-increment") {
        ucf::utilities::UnicodeString s = "ab";
        auto it = s.begin();
        auto old = it++;
        REQUIRE(*old == U'a');
        REQUIRE(*it == U'b');
    }
    
    SECTION("mixed byte lengths") {
        // a(1 byte) + 中(3 bytes) + 😀(4 bytes)
        ucf::utilities::UnicodeString s = "a" + ucf::utilities::UnicodeString(U'中') + ucf::utilities::UnicodeString(U'\U0001F600');
        std::vector<char32_t> chars;
        for (char32_t c : s) {
            chars.push_back(c);
        }
        REQUIRE(chars.size() == 3);
        REQUIRE(chars[0] == U'a');
        REQUIRE(chars[1] == U'中');
        REQUIRE(chars[2] == U'\U0001F600');
    }
}

//============================================
// 边界条件测试 - 比较操作
//============================================
TEST_CASE("UnicodeString - comparison edge cases", "[UnicodeString][EdgeCase]")
{
    SECTION("empty comparisons") {
        ucf::utilities::UnicodeString empty1, empty2;
        REQUIRE(empty1 == empty2);
        REQUIRE_FALSE(empty1 != empty2);
        REQUIRE_FALSE(empty1 < empty2);
        REQUIRE(empty1 <= empty2);
        REQUIRE_FALSE(empty1 > empty2);
        REQUIRE(empty1 >= empty2);
    }
    
    SECTION("empty vs non-empty") {
        ucf::utilities::UnicodeString empty;
        ucf::utilities::UnicodeString nonEmpty = "a";
        
        REQUIRE_FALSE(empty == nonEmpty);
        REQUIRE(empty != nonEmpty);
        REQUIRE(empty < nonEmpty);
        REQUIRE(nonEmpty > empty);
    }
    
    SECTION("same prefix different length") {
        ucf::utilities::UnicodeString s1 = "hello";
        ucf::utilities::UnicodeString s2 = "hello world";
        
        REQUIRE(s1 < s2);
        REQUIRE(s2 > s1);
        REQUIRE_FALSE(s1 == s2);
    }
    
    SECTION("unicode comparison (byte order)") {
        // UTF-8 字节序比较
        ucf::utilities::UnicodeString a = "a";      // 0x61
        ucf::utilities::UnicodeString z = "z";      // 0x7A
        ucf::utilities::UnicodeString cn = ucf::utilities::UnicodeString(U'中');  // 0xE4 0xB8 0xAD
        
        REQUIRE(a < z);
        REQUIRE(z < cn);  // ASCII < 多字节 UTF-8
    }
}

//============================================
// 边界条件测试 - 连接操作
//============================================
TEST_CASE("UnicodeString - concatenation edge cases", "[UnicodeString][EdgeCase]")
{
    SECTION("empty + empty") {
        ucf::utilities::UnicodeString a, b;
        REQUIRE((a + b).empty());
    }
    
    SECTION("empty + non-empty") {
        ucf::utilities::UnicodeString empty;
        ucf::utilities::UnicodeString s = "hello";
        REQUIRE(empty + s == "hello");
        REQUIRE(s + empty == "hello");
    }
    
    SECTION("append empty") {
        ucf::utilities::UnicodeString s = "hello";
        s += "";
        REQUIRE(s == "hello");
    }
    
    SECTION("append null char*") {
        ucf::utilities::UnicodeString s = "hello";
        s += static_cast<const char*>(nullptr);
        REQUIRE(s == "hello");
    }
    
    SECTION("long concatenation") {
        ucf::utilities::UnicodeString s;
        for (int i = 0; i < 1000; ++i) {
            s += "x";
        }
        REQUIRE(s.length() == 1000);
        REQUIRE(s.byteLength() == 1000);
    }
}

//============================================
// 边界条件测试 - fromWide
//============================================
TEST_CASE("UnicodeString - fromWide edge cases", "[UnicodeString][EdgeCase]")
{
    SECTION("empty wide string") {
        REQUIRE(ucf::utilities::UnicodeString::fromWide(L"").empty());
        REQUIRE(ucf::utilities::UnicodeString::fromWide(std::wstring()).empty());
    }
    
    SECTION("null pointer") {
        REQUIRE(ucf::utilities::UnicodeString::fromWide(static_cast<const wchar_t*>(nullptr)).empty());
    }
    
    SECTION("with length") {
        std::wstring wide = L"hello world";
        auto s = ucf::utilities::UnicodeString::fromWide(wide.c_str(), 5);
        REQUIRE(s == "hello");
    }
    
    SECTION("zero length") {
        auto s = ucf::utilities::UnicodeString::fromWide(L"hello", 0);
        REQUIRE(s.empty());
    }
}

//============================================
// 边界条件测试 - fromLocal
//============================================
TEST_CASE("UnicodeString - fromLocal edge cases", "[UnicodeString][EdgeCase]")
{
    SECTION("empty local string") {
        REQUIRE(ucf::utilities::UnicodeString::fromLocal("").empty());
        REQUIRE(ucf::utilities::UnicodeString::fromLocal(std::string()).empty());
    }
    
    SECTION("null pointer") {
        REQUIRE(ucf::utilities::UnicodeString::fromLocal(static_cast<const char*>(nullptr)).empty());
    }
    
    SECTION("ASCII via fromLocal") {
        // ASCII 在任何编码下都相同
        auto s = ucf::utilities::UnicodeString::fromLocal("hello");
        REQUIRE(s == "hello");
    }
}

//============================================
// 边界条件测试 - fromUtf16
//============================================
TEST_CASE("UnicodeString - fromUtf16 edge cases", "[UnicodeString][EdgeCase]")
{
    SECTION("empty") {
        REQUIRE(ucf::utilities::UnicodeString::fromUtf16(u"").empty());
        REQUIRE(ucf::utilities::UnicodeString::fromUtf16(std::u16string()).empty());
    }
    
    SECTION("null pointer") {
        REQUIRE(ucf::utilities::UnicodeString::fromUtf16(static_cast<const char16_t*>(nullptr)).empty());
    }
    
    SECTION("BMP characters only") {
        auto s = ucf::utilities::UnicodeString::fromUtf16(u"Hello世界");
        REQUIRE(s.length() == 7);
    }
    
    SECTION("surrogate pairs") {
        // 𝄞 = U+1D11E (需要 surrogate pair: D834 DD1E)
        std::u16string utf16 = u"\U0001D11E";
        auto s = ucf::utilities::UnicodeString::fromUtf16(utf16);
        REQUIRE(s.length() == 1);
        REQUIRE(s[0] == U'\U0001D11E');
    }
    
    SECTION("mixed BMP and SMP") {
        std::u16string utf16 = u"A\U0001F600B\U0001F601C";
        auto s = ucf::utilities::UnicodeString::fromUtf16(utf16);
        REQUIRE(s.length() == 5);
    }
}

//============================================
// 边界条件测试 - fromUtf32
//============================================
TEST_CASE("UnicodeString - fromUtf32 edge cases", "[UnicodeString][EdgeCase]")
{
    SECTION("empty") {
        REQUIRE(ucf::utilities::UnicodeString::fromUtf32(U"").empty());
        REQUIRE(ucf::utilities::UnicodeString::fromUtf32(std::u32string()).empty());
    }
    
    SECTION("null pointer") {
        REQUIRE(ucf::utilities::UnicodeString::fromUtf32(static_cast<const char32_t*>(nullptr)).empty());
    }
    
    SECTION("full range codepoints") {
        // 测试不同范围的码点
        std::u32string utf32;
        utf32 += U'\u0041';      // ASCII
        utf32 += U'\u00F1';      // Latin Extended
        utf32 += U'\u4E2D';      // CJK
        utf32 += U'\U0001F600';  // Emoji
        
        auto s = ucf::utilities::UnicodeString::fromUtf32(utf32);
        REQUIRE(s.length() == 4);
        REQUIRE(s[0] == U'\u0041');
        REQUIRE(s[1] == U'\u00F1');
        REQUIRE(s[2] == U'\u4E2D');
        REQUIRE(s[3] == U'\U0001F600');
    }
}

//============================================
// 边界条件测试 - 转换往返
//============================================
TEST_CASE("UnicodeString - conversion roundtrip", "[UnicodeString][EdgeCase]")
{
    // 使用各种字符测试往返转换
    ucf::utilities::UnicodeString original = "Hello" + ucf::utilities::UnicodeString::fromWide(L"世界") + 
                              ucf::utilities::UnicodeString::fromUtf32(U"\U0001F600");
    
    SECTION("toWide roundtrip") {
        auto wide = original.toWide();
        auto back = ucf::utilities::UnicodeString::fromWide(wide);
        REQUIRE(back == original);
    }
    
    SECTION("toUtf16 roundtrip") {
        auto utf16 = original.toUtf16();
        auto back = ucf::utilities::UnicodeString::fromUtf16(utf16);
        REQUIRE(back == original);
    }
    
    SECTION("toUtf32 roundtrip") {
        auto utf32 = original.toUtf32();
        auto back = ucf::utilities::UnicodeString::fromUtf32(utf32);
        REQUIRE(back == original);
    }
    
    SECTION("toLocal roundtrip (ASCII only)") {
        ucf::utilities::UnicodeString ascii = "Hello World 123";
        auto local = ascii.toLocal();
        auto back = ucf::utilities::UnicodeString::fromLocal(local);
        REQUIRE(back == ascii);
    }
}

//============================================
// 边界条件测试 - isValidUtf8
//============================================
TEST_CASE("UnicodeString - isValidUtf8 edge cases", "[UnicodeString][EdgeCase]")
{
    SECTION("valid sequences") {
        REQUIRE(ucf::utilities::UnicodeString::isValidUtf8(""));
        REQUIRE(ucf::utilities::UnicodeString::isValidUtf8("hello"));
        REQUIRE(ucf::utilities::UnicodeString::isValidUtf8(U8STR("你好")));
    }
    
    SECTION("with length") {
        std::string s = "hello";
        REQUIRE(ucf::utilities::UnicodeString::isValidUtf8(s.c_str(), s.length()));
        REQUIRE(ucf::utilities::UnicodeString::isValidUtf8(s.c_str(), 0));  // 空
    }
}

//============================================
// 边界条件测试 - clear 和 reserve
//============================================
TEST_CASE("UnicodeString - clear and reserve edge cases", "[UnicodeString][EdgeCase]")
{
    SECTION("clear empty string") {
        ucf::utilities::UnicodeString s;
        s.clear();
        REQUIRE(s.empty());
    }
    
    SECTION("clear then append") {
        ucf::utilities::UnicodeString s = "hello";
        s.clear();
        s += "world";
        REQUIRE(s == "world");
    }
    
    SECTION("reserve zero") {
        ucf::utilities::UnicodeString s = "hello";
        s.reserve(0);
        REQUIRE(s == "hello");  // 内容不变
    }
    
    SECTION("reserve large") {
        ucf::utilities::UnicodeString s;
        s.reserve(10000);
        REQUIRE(s.capacity() >= 10000);
        REQUIRE(s.empty());  // 内容仍为空
    }
    
    SECTION("capacity after clear") {
        ucf::utilities::UnicodeString s = "hello world this is a test";
        size_t capBefore = s.capacity();
        s.clear();
        REQUIRE(s.capacity() >= capBefore);  // capacity 通常保留
        REQUIRE(s.empty());
    }
}

//============================================
// 边界条件测试 - 隐式转换
//============================================
TEST_CASE("UnicodeString - implicit conversion", "[UnicodeString][EdgeCase]")
{
    SECTION("to const std::string&") {
        ucf::utilities::UnicodeString us = "hello";
        const std::string& ref = us;
        REQUIRE(ref == "hello");
    }
    
    SECTION("to std::string_view") {
        ucf::utilities::UnicodeString us = "hello";
        std::string_view view = us;
        REQUIRE(view == std::string_view("hello"));
    }
    
    SECTION("function accepting std::string") {
        ucf::utilities::UnicodeString us = "hello";
        auto testFunc = [](const std::string& s) { return s.length(); };
        REQUIRE(testFunc(us) == 5);
    }
}

//============================================
// 边界条件测试 - 长字符串
//============================================
TEST_CASE("UnicodeString - long strings", "[UnicodeString][EdgeCase]")
{
    SECTION("long ASCII") {
        std::string longStr(10000, 'x');
        ucf::utilities::UnicodeString s = longStr;
        REQUIRE(s.length() == 10000);
        REQUIRE(s.byteLength() == 10000);
        REQUIRE(s.isAscii());
    }
    
    SECTION("long Unicode") {
        std::u32string longUtf32(1000, U'中');
        ucf::utilities::UnicodeString s = ucf::utilities::UnicodeString::fromUtf32(longUtf32);
        REQUIRE(s.length() == 1000);
        REQUIRE(s.byteLength() == 3000);  // 每个中文 3 字节
    }
    
    SECTION("substr of long string") {
        std::string longStr(10000, 'x');
        ucf::utilities::UnicodeString s = longStr;
        auto sub = s.substr(5000, 100);
        REQUIRE(sub.length() == 100);
    }
}

//============================================
// StringUtils + UnicodeString 更多测试
//============================================
TEST_CASE("StringUtils with UnicodeString - ltrim/rtrim", "[UnicodeString][StringUtils]")
{
    ucf::utilities::UnicodeString s = ucf::utilities::UnicodeString::fromWide(L"  你好  ");
    REQUIRE(ucf::utilities::StringUtils::ltrim(s) == ucf::utilities::UnicodeString::fromWide(L"你好  "));
    REQUIRE(ucf::utilities::StringUtils::rtrim(s) == ucf::utilities::UnicodeString::fromWide(L"  你好"));
}

TEST_CASE("StringUtils with UnicodeString - split", "[UnicodeString][StringUtils]")
{
    ucf::utilities::UnicodeString s = ucf::utilities::UnicodeString::fromWide(L"北京,上海,广州");
    auto parts = ucf::utilities::StringUtils::split(s, ',');
    REQUIRE(parts.size() == 3);
    REQUIRE(parts[0] == ucf::utilities::UnicodeString::fromWide(L"北京"));
    REQUIRE(parts[1] == ucf::utilities::UnicodeString::fromWide(L"上海"));
    REQUIRE(parts[2] == ucf::utilities::UnicodeString::fromWide(L"广州"));
}

TEST_CASE("StringUtils with UnicodeString - join", "[UnicodeString][StringUtils]")
{
    std::vector<ucf::utilities::UnicodeString> parts = {
        ucf::utilities::UnicodeString::fromWide(L"北京"),
        ucf::utilities::UnicodeString::fromWide(L"上海"),
        ucf::utilities::UnicodeString::fromWide(L"广州")
    };
    auto joined = ucf::utilities::StringUtils::join(parts, ucf::utilities::UnicodeString::fromWide(L"、"));
    REQUIRE(joined == ucf::utilities::UnicodeString::fromWide(L"北京、上海、广州"));
}

TEST_CASE("StringUtils with UnicodeString - replace", "[UnicodeString][StringUtils]")
{
    ucf::utilities::UnicodeString s = ucf::utilities::UnicodeString::fromWide(L"你好世界");
    auto result = ucf::utilities::StringUtils::replace(s, 
                                       ucf::utilities::UnicodeString::fromWide(L"世界"), 
                                       ucf::utilities::UnicodeString::fromWide(L"中国"));
    REQUIRE(result == ucf::utilities::UnicodeString::fromWide(L"你好中国"));
}

TEST_CASE("StringUtils with UnicodeString - replaceAll", "[UnicodeString][StringUtils]")
{
    ucf::utilities::UnicodeString s = ucf::utilities::UnicodeString::fromWide(L"一二一二一");
    auto result = ucf::utilities::StringUtils::replaceAll(s, 
                                          ucf::utilities::UnicodeString::fromWide(L"一"), 
                                          ucf::utilities::UnicodeString::fromWide(L"壹"));
    REQUIRE(result == ucf::utilities::UnicodeString::fromWide(L"壹二壹二壹"));
}

//============================================
// 特殊 Unicode 字符测试
//============================================
TEST_CASE("UnicodeString - special unicode characters", "[UnicodeString][Unicode]")
{
    SECTION("zero-width characters") {
        // Zero-width space U+200B
        ucf::utilities::UnicodeString s = ucf::utilities::UnicodeString::fromUtf32(U"\u200B");
        REQUIRE(s.length() == 1);
        REQUIRE_FALSE(s.empty());
    }
    
    SECTION("combining characters") {
        // e + combining acute accent = é
        // 注意：这是两个码点，不是一个
        std::u32string combining = U"e\u0301";
        ucf::utilities::UnicodeString s = ucf::utilities::UnicodeString::fromUtf32(combining);
        REQUIRE(s.length() == 2);  // 2 个码点
    }
    
    SECTION("right-to-left text") {
        // Arabic text
        ucf::utilities::UnicodeString s = ucf::utilities::UnicodeString::fromUtf32(U"\u0645\u0631\u062D\u0628\u0627");
        REQUIRE(s.length() == 5);
        REQUIRE(s.isValid());
    }
    
    SECTION("mathematical symbols") {
        // ∑ ∫ √
        ucf::utilities::UnicodeString s = ucf::utilities::UnicodeString::fromUtf32(U"\u2211\u222B\u221A");
        REQUIRE(s.length() == 3);
    }
}

TEST_CASE("UnicodeString - size vs length", "[UnicodeString]")
{
    // size() 和 length() 应该返回相同的值
    ucf::utilities::UnicodeString s = ucf::utilities::UnicodeString::fromWide(L"你好Hello");
    REQUIRE(s.size() == s.length());
    REQUIRE(s.byteSize() == s.byteLength());
    
    REQUIRE(s.size() == 7);
    REQUIRE(s.byteSize() == 11);  // 2*3 + 5 = 11
}
