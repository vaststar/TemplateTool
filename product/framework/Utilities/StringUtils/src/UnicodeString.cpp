#include <ucf/Utilities/StringUtils/UnicodeString.h>
#include "UnicodeStringEncoding.h"

#include <stdexcept>
#include <ostream>

namespace ucf::utilities {

// ==========================================
// 构造函数
// ==========================================

UnicodeString::UnicodeString() = default;

UnicodeString::UnicodeString(const std::string& utf8)
    : mData(utf8)
{
}

UnicodeString::UnicodeString(const char* utf8)
    : mData(utf8 ? utf8 : "")
{
}

UnicodeString::UnicodeString(std::string_view utf8)
    : mData(utf8)
{
}

UnicodeString::UnicodeString(char32_t codepoint)
{
    detail::encodeUtf8(codepoint, mData);
}

UnicodeString::~UnicodeString() = default;

UnicodeString::UnicodeString(const UnicodeString& other) = default;
UnicodeString& UnicodeString::operator=(const UnicodeString& other) = default;
UnicodeString::UnicodeString(UnicodeString&& other) noexcept = default;
UnicodeString& UnicodeString::operator=(UnicodeString&& other) noexcept = default;

// ==========================================
// 静态工厂方法
// ==========================================

UnicodeString UnicodeString::fromWide(const std::wstring& wide)
{
    return UnicodeString(detail::wideToUtf8(wide));
}

UnicodeString UnicodeString::fromWide(const wchar_t* wide)
{
    if (!wide) return UnicodeString();
    return fromWide(std::wstring(wide));
}

UnicodeString UnicodeString::fromWide(const wchar_t* wide, size_t length)
{
    if (!wide) return UnicodeString();
    return UnicodeString(detail::wideToUtf8(wide, length));
}

UnicodeString UnicodeString::fromLocal(const std::string& local)
{
    return UnicodeString(detail::localToUtf8(local));
}

UnicodeString UnicodeString::fromLocal(const char* local)
{
    if (!local) return UnicodeString();
    return fromLocal(std::string(local));
}

UnicodeString UnicodeString::fromUtf8(const std::string& utf8)
{
    return UnicodeString(utf8);
}

UnicodeString UnicodeString::fromUtf8(const char* utf8)
{
    return UnicodeString(utf8);
}

UnicodeString UnicodeString::fromUtf16(const std::u16string& utf16)
{
    return UnicodeString(detail::utf16ToUtf8(utf16));
}

UnicodeString UnicodeString::fromUtf16(const char16_t* utf16)
{
    if (!utf16) return UnicodeString();
    return fromUtf16(std::u16string(utf16));
}

UnicodeString UnicodeString::fromUtf32(const std::u32string& utf32)
{
    return UnicodeString(detail::utf32ToUtf8(utf32));
}

UnicodeString UnicodeString::fromUtf32(const char32_t* utf32)
{
    if (!utf32) return UnicodeString();
    return fromUtf32(std::u32string(utf32));
}

// ==========================================
// 转换输出
// ==========================================

const std::string& UnicodeString::toStdString() const noexcept
{
    return mData;
}

std::string_view UnicodeString::toStringView() const noexcept
{
    return mData;
}

const char* UnicodeString::c_str() const noexcept
{
    return mData.c_str();
}

const char* UnicodeString::data() const noexcept
{
    return mData.data();
}

std::wstring UnicodeString::toWide() const
{
    return detail::utf8ToWide(mData);
}

std::string UnicodeString::toLocal() const
{
    return detail::utf8ToLocal(mData);
}

std::u16string UnicodeString::toUtf16() const
{
    return detail::utf8ToUtf16(mData);
}

std::u32string UnicodeString::toUtf32() const
{
    return detail::utf8ToUtf32(mData);
}

// ==========================================
// 隐式转换
// ==========================================

UnicodeString::operator const std::string&() const noexcept
{
    return mData;
}

UnicodeString::operator std::string_view() const noexcept
{
    return mData;
}

// ==========================================
// 长度和容量
// ==========================================

size_t UnicodeString::length() const
{
    return detail::countCodepoints(mData);
}

size_t UnicodeString::size() const
{
    return length();
}

size_t UnicodeString::byteLength() const noexcept
{
    return mData.size();
}

size_t UnicodeString::byteSize() const noexcept
{
    return mData.size();
}

bool UnicodeString::empty() const noexcept
{
    return mData.empty();
}

void UnicodeString::clear() noexcept
{
    mData.clear();
}

void UnicodeString::reserve(size_t bytes)
{
    mData.reserve(bytes);
}

size_t UnicodeString::capacity() const noexcept
{
    return mData.capacity();
}

// ==========================================
// 字符访问
// ==========================================

const char* UnicodeString::pointerAt(size_t charIndex) const
{
    return detail::codepointPointer(mData, charIndex);
}

char32_t UnicodeString::at(size_t index) const
{
    size_t len = length();
    if (index >= len) {
        throw std::out_of_range("UnicodeString::at: index out of range");
    }
    const char* ptr = pointerAt(index);
    const char* end = mData.data() + mData.size();
    return detail::decodeUtf8(ptr, end);
}

char32_t UnicodeString::operator[](size_t index) const
{
    const char* ptr = pointerAt(index);
    const char* end = mData.data() + mData.size();
    return detail::decodeUtf8(ptr, end);
}

char32_t UnicodeString::front() const
{
    if (empty()) {
        throw std::out_of_range("UnicodeString::front: string is empty");
    }
    return (*this)[0];
}

char32_t UnicodeString::back() const
{
    size_t len = length();
    if (len == 0) {
        throw std::out_of_range("UnicodeString::back: string is empty");
    }
    return (*this)[len - 1];
}

// ==========================================
// 迭代器
// ==========================================

UnicodeString::ConstIterator::ConstIterator() noexcept
    : mPtr(nullptr), mEnd(nullptr)
{
}

UnicodeString::ConstIterator::ConstIterator(const char* ptr, const char* end) noexcept
    : mPtr(ptr), mEnd(end)
{
}

char32_t UnicodeString::ConstIterator::operator*() const
{
    const char* temp = mPtr;
    return detail::decodeUtf8(temp, mEnd);
}

UnicodeString::ConstIterator& UnicodeString::ConstIterator::operator++()
{
    detail::decodeUtf8(mPtr, mEnd);
    return *this;
}

UnicodeString::ConstIterator UnicodeString::ConstIterator::operator++(int)
{
    ConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

bool UnicodeString::ConstIterator::operator==(const ConstIterator& other) const noexcept
{
    return mPtr == other.mPtr;
}

bool UnicodeString::ConstIterator::operator!=(const ConstIterator& other) const noexcept
{
    return mPtr != other.mPtr;
}

UnicodeString::ConstIterator UnicodeString::begin() const noexcept
{
    return ConstIterator(mData.data(), mData.data() + mData.size());
}

UnicodeString::ConstIterator UnicodeString::end() const noexcept
{
    const char* endPtr = mData.data() + mData.size();
    return ConstIterator(endPtr, endPtr);
}

UnicodeString::ConstIterator UnicodeString::cbegin() const noexcept
{
    return begin();
}

UnicodeString::ConstIterator UnicodeString::cend() const noexcept
{
    return end();
}

// ==========================================
// 运算符
// ==========================================

UnicodeString UnicodeString::operator+(const UnicodeString& other) const
{
    return UnicodeString(mData + other.mData);
}

UnicodeString UnicodeString::operator+(const std::string& utf8) const
{
    return UnicodeString(mData + utf8);
}

UnicodeString UnicodeString::operator+(const char* utf8) const
{
    return UnicodeString(mData + (utf8 ? utf8 : ""));
}

UnicodeString UnicodeString::operator+(char32_t codepoint) const
{
    UnicodeString result = *this;
    result += codepoint;
    return result;
}

UnicodeString& UnicodeString::operator+=(const UnicodeString& other)
{
    mData += other.mData;
    return *this;
}

UnicodeString& UnicodeString::operator+=(const std::string& utf8)
{
    mData += utf8;
    return *this;
}

UnicodeString& UnicodeString::operator+=(const char* utf8)
{
    if (utf8) mData += utf8;
    return *this;
}

UnicodeString& UnicodeString::operator+=(char32_t codepoint)
{
    detail::encodeUtf8(codepoint, mData);
    return *this;
}

bool UnicodeString::operator==(const UnicodeString& other) const noexcept
{
    return mData == other.mData;
}

bool UnicodeString::operator!=(const UnicodeString& other) const noexcept
{
    return mData != other.mData;
}

bool UnicodeString::operator<(const UnicodeString& other) const noexcept
{
    return mData < other.mData;
}

bool UnicodeString::operator<=(const UnicodeString& other) const noexcept
{
    return mData <= other.mData;
}

bool UnicodeString::operator>(const UnicodeString& other) const noexcept
{
    return mData > other.mData;
}

bool UnicodeString::operator>=(const UnicodeString& other) const noexcept
{
    return mData >= other.mData;
}

bool UnicodeString::operator==(const std::string& other) const noexcept
{
    return mData == other;
}

bool UnicodeString::operator==(const char* other) const noexcept
{
    if (!other) return mData.empty();
    return mData == other;
}

// ==========================================
// 验证
// ==========================================

bool UnicodeString::isValid() const
{
    return detail::validateUtf8(mData.data(), mData.size());
}

bool UnicodeString::isAscii() const
{
    return detail::checkAscii(mData.data(), mData.size());
}

bool UnicodeString::isValidUtf8(const std::string& str)
{
    return detail::validateUtf8(str.data(), str.size());
}

bool UnicodeString::isValidUtf8(const char* str, size_t length)
{
    if (!str) return length == 0;
    return detail::validateUtf8(str, length);
}

// ==========================================
// 子串
// ==========================================

UnicodeString UnicodeString::substr(size_t pos, size_t count) const
{
    size_t len = length();
    if (pos >= len) {
        return UnicodeString();
    }
    
    const char* start = pointerAt(pos);
    
    if (count == npos || pos + count >= len) {
        return UnicodeString(std::string(start, mData.data() + mData.size() - start));
    }
    
    const char* endPtr = pointerAt(pos + count);
    return UnicodeString(std::string(start, endPtr - start));
}

// ==========================================
// 非成员函数
// ==========================================

std::ostream& operator<<(std::ostream& os, const UnicodeString& str)
{
    return os << str.toStdString();
}

UnicodeString operator+(const char* lhs, const UnicodeString& rhs)
{
    return UnicodeString(lhs) + rhs;
}

UnicodeString operator+(const std::string& lhs, const UnicodeString& rhs)
{
    return UnicodeString(lhs) + rhs;
}

bool operator==(const std::string& lhs, const UnicodeString& rhs) noexcept
{
    return rhs == lhs;
}

bool operator==(const char* lhs, const UnicodeString& rhs) noexcept
{
    return rhs == lhs;
}

} // namespace ucf::utilities
