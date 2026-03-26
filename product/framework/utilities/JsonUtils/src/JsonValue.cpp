#include <ucf/Utilities/JsonUtils/JsonValue.h>

#include <nlohmann/json.hpp>

#include <cmath>
#include <limits>
#include <stdexcept>

namespace ucf::utilities {

// ==========================================
// PIMPL Implementation
// ==========================================

class JsonValue::Impl
{
public:
    nlohmann::json json;

    Impl() : json(nullptr) {}
    explicit Impl(nlohmann::json j) : json(std::move(j)) {}
};

// ==========================================
// ConstIterator Implementation
// ==========================================

class JsonValue::ConstIterator::Impl
{
public:
    nlohmann::json::const_iterator iter;
    nlohmann::json::const_iterator end;
    bool valid = false;

    Impl() = default;
    Impl(nlohmann::json::const_iterator it, nlohmann::json::const_iterator endIt)
        : iter(it), end(endIt), valid(true) {}
};

JsonValue::ConstIterator::ConstIterator()
    : mImpl(std::make_unique<Impl>())
{
}

JsonValue::ConstIterator::~ConstIterator() = default;

JsonValue::ConstIterator::ConstIterator(const ConstIterator& other)
    : mImpl(std::make_unique<Impl>(*other.mImpl))
{
}

JsonValue::ConstIterator::ConstIterator(ConstIterator&& other) noexcept = default;

JsonValue::ConstIterator& JsonValue::ConstIterator::operator=(const ConstIterator& other)
{
    if (this != &other)
    {
        mImpl = std::make_unique<Impl>(*other.mImpl);
    }
    return *this;
}

JsonValue::ConstIterator& JsonValue::ConstIterator::operator=(ConstIterator&& other) noexcept = default;

JsonValue::ConstIterator::ConstIterator(std::unique_ptr<Impl> impl)
    : mImpl(std::move(impl))
{
}

JsonValue JsonValue::ConstIterator::operator*() const
{
    auto valueImpl = std::make_unique<JsonValue::Impl>(*mImpl->iter);
    return JsonValue(std::move(valueImpl));
}

JsonValue::ConstIterator& JsonValue::ConstIterator::operator++()
{
    ++mImpl->iter;
    return *this;
}

JsonValue::ConstIterator JsonValue::ConstIterator::operator++(int)
{
    ConstIterator tmp(*this);
    ++(*this);
    return tmp;
}

bool JsonValue::ConstIterator::operator==(const ConstIterator& other) const
{
    // Both invalid iterators are equal (both represent "end")
    if (!mImpl->valid && !other.mImpl->valid)
    {
        return true;
    }
    // One valid, one invalid - not equal
    if (mImpl->valid != other.mImpl->valid)
    {
        return false;
    }
    // Both valid - compare underlying iterators
    return mImpl->iter == other.mImpl->iter;
}

bool JsonValue::ConstIterator::operator!=(const ConstIterator& other) const
{
    return !(*this == other);
}

// ==========================================
// ObjectIterator Implementation
// ==========================================

class JsonValue::ObjectIterator::Impl
{
public:
    nlohmann::json::const_iterator iter;
    nlohmann::json::const_iterator end;
    bool valid = false;

    Impl() = default;
    Impl(nlohmann::json::const_iterator it, nlohmann::json::const_iterator endIt)
        : iter(it), end(endIt), valid(true) {}
};

JsonValue::ObjectIterator::ObjectIterator()
    : mImpl(std::make_unique<Impl>())
{
}

JsonValue::ObjectIterator::~ObjectIterator() = default;

JsonValue::ObjectIterator::ObjectIterator(const ObjectIterator& other)
    : mImpl(std::make_unique<Impl>(*other.mImpl))
{
}

JsonValue::ObjectIterator::ObjectIterator(ObjectIterator&& other) noexcept = default;

JsonValue::ObjectIterator& JsonValue::ObjectIterator::operator=(const ObjectIterator& other)
{
    if (this != &other)
    {
        mImpl = std::make_unique<Impl>(*other.mImpl);
    }
    return *this;
}

JsonValue::ObjectIterator& JsonValue::ObjectIterator::operator=(ObjectIterator&& other) noexcept = default;

JsonValue::ObjectIterator::ObjectIterator(std::unique_ptr<Impl> impl)
    : mImpl(std::move(impl))
{
}

JsonValue::ObjectIterator::value_type JsonValue::ObjectIterator::operator*() const
{
    auto valueImpl = std::make_unique<JsonValue::Impl>(mImpl->iter.value());
    return {mImpl->iter.key(), JsonValue(std::move(valueImpl))};
}

JsonValue::ObjectIterator& JsonValue::ObjectIterator::operator++()
{
    ++mImpl->iter;
    return *this;
}

JsonValue::ObjectIterator JsonValue::ObjectIterator::operator++(int)
{
    ObjectIterator tmp(*this);
    ++(*this);
    return tmp;
}

bool JsonValue::ObjectIterator::operator==(const ObjectIterator& other) const
{
    // Both invalid iterators are equal (both represent "end")
    if (!mImpl->valid && !other.mImpl->valid)
    {
        return true;
    }
    // One valid, one invalid - not equal
    if (mImpl->valid != other.mImpl->valid)
    {
        return false;
    }
    // Both valid - compare underlying iterators
    return mImpl->iter == other.mImpl->iter;
}

bool JsonValue::ObjectIterator::operator!=(const ObjectIterator& other) const
{
    return !(*this == other);
}

// ==========================================
// ObjectRange Implementation
// ==========================================

JsonValue::ObjectRange::ObjectRange(ObjectIterator beginIt, ObjectIterator endIt)
    : mBegin(std::move(beginIt)), mEnd(std::move(endIt))
{
}

JsonValue::ObjectRange::ObjectRange(const ObjectRange& other)
    : mBegin(other.mBegin), mEnd(other.mEnd)
{
}

JsonValue::ObjectRange::ObjectRange(ObjectRange&& other) noexcept
    : mBegin(std::move(other.mBegin)), mEnd(std::move(other.mEnd))
{
}

JsonValue::ObjectRange& JsonValue::ObjectRange::operator=(const ObjectRange& other)
{
    if (this != &other)
    {
        mBegin = other.mBegin;
        mEnd = other.mEnd;
    }
    return *this;
}

JsonValue::ObjectRange& JsonValue::ObjectRange::operator=(ObjectRange&& other) noexcept
{
    if (this != &other)
    {
        mBegin = std::move(other.mBegin);
        mEnd = std::move(other.mEnd);
    }
    return *this;
}

JsonValue::ObjectRange::~ObjectRange() = default;

JsonValue::ObjectIterator JsonValue::ObjectRange::begin() const
{
    return mBegin;
}

JsonValue::ObjectIterator JsonValue::ObjectRange::end() const
{
    return mEnd;
}

// ==========================================
// JsonValue Constructors
// ==========================================

JsonValue::JsonValue()
    : mImpl(std::make_unique<Impl>())
{
}

JsonValue::JsonValue(std::nullptr_t)
    : mImpl(std::make_unique<Impl>())
{
}

JsonValue::JsonValue(bool value)
    : mImpl(std::make_unique<Impl>(nlohmann::json(value)))
{
}

JsonValue::JsonValue(int32_t value)
    : mImpl(std::make_unique<Impl>(nlohmann::json(value)))
{
}

JsonValue::JsonValue(int64_t value)
    : mImpl(std::make_unique<Impl>(nlohmann::json(value)))
{
}

JsonValue::JsonValue(uint32_t value)
    : mImpl(std::make_unique<Impl>(nlohmann::json(value)))
{
}

JsonValue::JsonValue(uint64_t value)
    : mImpl(std::make_unique<Impl>(nlohmann::json(value)))
{
}

JsonValue::JsonValue(double value)
    : mImpl(std::make_unique<Impl>(nlohmann::json(value)))
{
}

JsonValue::JsonValue(const std::string& value)
    : mImpl(std::make_unique<Impl>(nlohmann::json(value)))
{
}

JsonValue::JsonValue(std::string&& value)
    : mImpl(std::make_unique<Impl>(nlohmann::json(std::move(value))))
{
}

JsonValue::JsonValue(const char* value)
    : mImpl(std::make_unique<Impl>(nlohmann::json(value)))
{
}

JsonValue::JsonValue(std::vector<JsonValue> arr)
    : mImpl(std::make_unique<Impl>(nlohmann::json::array()))
{
    for (auto& item : arr)
    {
        mImpl->json.push_back(std::move(item.mImpl->json));
    }
}

JsonValue::JsonValue(std::map<std::string, JsonValue> obj)
    : mImpl(std::make_unique<Impl>(nlohmann::json::object()))
{
    for (auto& [key, value] : obj)
    {
        mImpl->json[key] = std::move(value.mImpl->json);
    }
}

JsonValue::JsonValue(std::unique_ptr<Impl> impl)
    : mImpl(std::move(impl))
{
}

// ==========================================
// Copy & Move
// ==========================================

JsonValue::JsonValue(const JsonValue& other)
    : mImpl(std::make_unique<Impl>(other.mImpl->json))
{
}

JsonValue::JsonValue(JsonValue&& other) noexcept = default;

JsonValue& JsonValue::operator=(const JsonValue& other)
{
    if (this != &other)
    {
        mImpl = std::make_unique<Impl>(other.mImpl->json);
    }
    return *this;
}

JsonValue& JsonValue::operator=(JsonValue&& other) noexcept = default;

JsonValue::~JsonValue() = default;

// ==========================================
// Type Query
// ==========================================

JsonType JsonValue::type() const
{
    if (mImpl->json.is_null()) return JsonType::Null;
    if (mImpl->json.is_boolean()) return JsonType::Bool;
    if (mImpl->json.is_number()) return JsonType::Number;
    if (mImpl->json.is_string()) return JsonType::String;
    if (mImpl->json.is_array()) return JsonType::Array;
    if (mImpl->json.is_object()) return JsonType::Object;
    return JsonType::Null;
}

bool JsonValue::isNull() const
{
    return mImpl->json.is_null();
}

bool JsonValue::isBool() const
{
    return mImpl->json.is_boolean();
}

bool JsonValue::isNumber() const
{
    return mImpl->json.is_number();
}

bool JsonValue::isString() const
{
    return mImpl->json.is_string();
}

bool JsonValue::isArray() const
{
    return mImpl->json.is_array();
}

bool JsonValue::isObject() const
{
    return mImpl->json.is_object();
}

bool JsonValue::isInteger() const
{
    if (!isNumber()) return false;
    if (mImpl->json.is_number_integer()) return true;
    // Check if float has no fractional part
    if (mImpl->json.is_number_float())
    {
        double val = mImpl->json.get<double>();
        if (!std::isfinite(val)) return false;
        double intPart;
        return std::modf(val, &intPart) == 0.0;
    }
    return false;
}

bool JsonValue::isFloat() const
{
    if (!isNumber()) return false;
    return !isInteger();
}

// ==========================================
// Value Getters
// ==========================================

std::optional<bool> JsonValue::asBool() const
{
    if (!isBool()) return std::nullopt;
    return mImpl->json.get<bool>();
}

std::optional<int32_t> JsonValue::asInt32() const
{
    if (!isInteger()) return std::nullopt;
    
    int64_t val = 0;
    if (mImpl->json.is_number_integer())
    {
        val = mImpl->json.get<int64_t>();
    }
    else if (mImpl->json.is_number_float())
    {
        double d = mImpl->json.get<double>();
        val = static_cast<int64_t>(d);
    }
    
    if (val < std::numeric_limits<int32_t>::min() ||
        val > std::numeric_limits<int32_t>::max())
    {
        return std::nullopt;
    }
    return static_cast<int32_t>(val);
}

std::optional<int64_t> JsonValue::asInt64() const
{
    if (!isInteger()) return std::nullopt;
    
    if (mImpl->json.is_number_unsigned())
    {
        uint64_t val = mImpl->json.get<uint64_t>();
        if (val > static_cast<uint64_t>(std::numeric_limits<int64_t>::max()))
        {
            return std::nullopt;
        }
        return static_cast<int64_t>(val);
    }
    
    if (mImpl->json.is_number_integer())
    {
        return mImpl->json.get<int64_t>();
    }
    
    if (mImpl->json.is_number_float())
    {
        double d = mImpl->json.get<double>();
        if (d < static_cast<double>(std::numeric_limits<int64_t>::min()) ||
            d > static_cast<double>(std::numeric_limits<int64_t>::max()))
        {
            return std::nullopt;
        }
        return static_cast<int64_t>(d);
    }
    
    return std::nullopt;
}

std::optional<uint32_t> JsonValue::asUInt32() const
{
    if (!isInteger()) return std::nullopt;
    
    if (mImpl->json.is_number_integer() && !mImpl->json.is_number_unsigned())
    {
        int64_t val = mImpl->json.get<int64_t>();
        if (val < 0 || val > std::numeric_limits<uint32_t>::max())
        {
            return std::nullopt;
        }
        return static_cast<uint32_t>(val);
    }
    
    if (mImpl->json.is_number_unsigned())
    {
        uint64_t val = mImpl->json.get<uint64_t>();
        if (val > std::numeric_limits<uint32_t>::max())
        {
            return std::nullopt;
        }
        return static_cast<uint32_t>(val);
    }
    
    if (mImpl->json.is_number_float())
    {
        double d = mImpl->json.get<double>();
        if (d < 0 || d > std::numeric_limits<uint32_t>::max())
        {
            return std::nullopt;
        }
        return static_cast<uint32_t>(d);
    }
    
    return std::nullopt;
}

std::optional<uint64_t> JsonValue::asUInt64() const
{
    if (!isInteger()) return std::nullopt;
    
    if (mImpl->json.is_number_integer() && !mImpl->json.is_number_unsigned())
    {
        int64_t val = mImpl->json.get<int64_t>();
        if (val < 0)
        {
            return std::nullopt;
        }
        return static_cast<uint64_t>(val);
    }
    
    if (mImpl->json.is_number_unsigned())
    {
        return mImpl->json.get<uint64_t>();
    }
    
    if (mImpl->json.is_number_float())
    {
        double d = mImpl->json.get<double>();
        if (d < 0 || d > static_cast<double>(std::numeric_limits<uint64_t>::max()))
        {
            return std::nullopt;
        }
        return static_cast<uint64_t>(d);
    }
    
    return std::nullopt;
}

std::optional<double> JsonValue::asDouble() const
{
    if (!isNumber()) return std::nullopt;
    return mImpl->json.get<double>();
}

std::optional<std::string> JsonValue::asString() const
{
    if (!isString()) return std::nullopt;
    return mImpl->json.get<std::string>();
}

// ==========================================
// Array Operations
// ==========================================

size_t JsonValue::size() const
{
    if (isArray() || isObject())
    {
        return mImpl->json.size();
    }
    return 0;
}

bool JsonValue::empty() const
{
    if (isArray() || isObject())
    {
        return mImpl->json.empty();
    }
    return true;
}

JsonValue JsonValue::get(size_t index) const
{
    if (!isArray() || index >= mImpl->json.size())
    {
        return JsonValue(); // Return null
    }
    auto impl = std::make_unique<Impl>(mImpl->json[index]);
    return JsonValue(std::move(impl));
}

JsonValue JsonValue::at(size_t index) const
{
    if (!isArray())
    {
        throw std::out_of_range("JsonValue is not an array");
    }
    if (index >= mImpl->json.size())
    {
        throw std::out_of_range("Array index out of range");
    }
    auto impl = std::make_unique<Impl>(mImpl->json[index]);
    return JsonValue(std::move(impl));
}

JsonValue JsonValue::operator[](size_t index) const
{
    return get(index);
}

void JsonValue::push_back(const JsonValue& value)
{
    // Convert to array if not already
    if (!isArray())
    {
        mImpl->json = nlohmann::json::array();
    }
    mImpl->json.push_back(value.mImpl->json);
}

void JsonValue::push_back(JsonValue&& value)
{
    // Convert to array if not already
    if (!isArray())
    {
        mImpl->json = nlohmann::json::array();
    }
    mImpl->json.push_back(std::move(value.mImpl->json));
}

// ==========================================
// Object Operations
// ==========================================

bool JsonValue::contains(const std::string& key) const
{
    return isObject() && mImpl->json.contains(key);
}

JsonValue JsonValue::get(const std::string& key) const
{
    if (!isObject() || !mImpl->json.contains(key))
    {
        return JsonValue(); // Return null
    }
    auto impl = std::make_unique<Impl>(mImpl->json[key]);
    return JsonValue(std::move(impl));
}

JsonValue JsonValue::at(const std::string& key) const
{
    if (!isObject())
    {
        throw std::out_of_range("JsonValue is not an object");
    }
    if (!mImpl->json.contains(key))
    {
        throw std::out_of_range("Key not found: " + key);
    }
    auto impl = std::make_unique<Impl>(mImpl->json[key]);
    return JsonValue(std::move(impl));
}

JsonValue JsonValue::operator[](const std::string& key) const
{
    return get(key);
}

JsonValue JsonValue::operator[](const char* key) const
{
    return (*this)[std::string(key)];
}

std::vector<std::string> JsonValue::keys() const
{
    std::vector<std::string> result;
    if (isObject())
    {
        result.reserve(mImpl->json.size());
        for (auto it = mImpl->json.begin(); it != mImpl->json.end(); ++it)
        {
            result.push_back(it.key());
        }
    }
    return result;
}

void JsonValue::set(const std::string& key, const JsonValue& value)
{
    // Convert to object if not already
    if (!isObject())
    {
        mImpl->json = nlohmann::json::object();
    }
    mImpl->json[key] = value.mImpl->json;
}

void JsonValue::set(const std::string& key, JsonValue&& value)
{
    // Convert to object if not already
    if (!isObject())
    {
        mImpl->json = nlohmann::json::object();
    }
    mImpl->json[key] = std::move(value.mImpl->json);
}

bool JsonValue::erase(const std::string& key)
{
    if (isObject() && mImpl->json.contains(key))
    {
        mImpl->json.erase(key);
        return true;
    }
    return false;
}

// ==========================================
// Array Conversion
// ==========================================

std::vector<JsonValue> JsonValue::toArray() const
{
    std::vector<JsonValue> result;
    if (isArray())
    {
        result.reserve(mImpl->json.size());
        for (const auto& item : mImpl->json)
        {
            auto impl = std::make_unique<Impl>(item);
            result.push_back(JsonValue(std::move(impl)));
        }
    }
    return result;
}

std::map<std::string, JsonValue> JsonValue::toMap() const
{
    std::map<std::string, JsonValue> result;
    if (isObject())
    {
        for (auto it = mImpl->json.begin(); it != mImpl->json.end(); ++it)
        {
            auto impl = std::make_unique<Impl>(it.value());
            result[it.key()] = JsonValue(std::move(impl));
        }
    }
    return result;
}

// ==========================================
// Iterators
// ==========================================

JsonValue::ConstIterator JsonValue::begin() const
{
    if (!isArray())
    {
        return end();
    }
    auto impl = std::make_unique<ConstIterator::Impl>(mImpl->json.begin(), mImpl->json.end());
    return ConstIterator(std::move(impl));
}

JsonValue::ConstIterator JsonValue::end() const
{
    if (!isArray())
    {
        auto impl = std::make_unique<ConstIterator::Impl>();
        return ConstIterator(std::move(impl));
    }
    auto impl = std::make_unique<ConstIterator::Impl>(mImpl->json.end(), mImpl->json.end());
    return ConstIterator(std::move(impl));
}

JsonValue::ObjectRange JsonValue::items() const
{
    if (!isObject())
    {
        auto beginImpl = std::make_unique<ObjectIterator::Impl>();
        auto endImpl = std::make_unique<ObjectIterator::Impl>();
        return ObjectRange(ObjectIterator(std::move(beginImpl)), ObjectIterator(std::move(endImpl)));
    }
    
    auto beginImpl = std::make_unique<ObjectIterator::Impl>(mImpl->json.begin(), mImpl->json.end());
    auto endImpl = std::make_unique<ObjectIterator::Impl>(mImpl->json.end(), mImpl->json.end());
    return ObjectRange(ObjectIterator(std::move(beginImpl)), ObjectIterator(std::move(endImpl)));
}

// ==========================================
// Serialization
// ==========================================

std::string JsonValue::dump() const
{
    return mImpl->json.dump();
}

std::string JsonValue::dumpPretty(int indent) const
{
    return mImpl->json.dump(indent);
}

// ==========================================
// Static Factory Methods
// ==========================================

JsonValue JsonValue::parse(const std::string& jsonStr)
{
    try
    {
        auto impl = std::make_unique<Impl>(nlohmann::json::parse(jsonStr));
        return JsonValue(std::move(impl));
    }
    catch (const nlohmann::json::parse_error&)
    {
        return JsonValue(); // Return null
    }
}

JsonValue::ParseResult JsonValue::parseEx(const std::string& jsonStr)
{
    ParseResult result;
    try
    {
        auto impl = std::make_unique<Impl>(nlohmann::json::parse(jsonStr));
        result.value = JsonValue(std::move(impl));
    }
    catch (const nlohmann::json::parse_error& e)
    {
        result.error = e.what();
    }
    return result;
}

JsonValue JsonValue::array()
{
    auto impl = std::make_unique<Impl>(nlohmann::json::array());
    return JsonValue(std::move(impl));
}

JsonValue JsonValue::array(std::vector<JsonValue> arr)
{
    return JsonValue(std::move(arr));
}

JsonValue JsonValue::object()
{
    auto impl = std::make_unique<Impl>(nlohmann::json::object());
    return JsonValue(std::move(impl));
}

JsonValue JsonValue::object(std::map<std::string, JsonValue> obj)
{
    return JsonValue(std::move(obj));
}

// ==========================================
// Comparison Operators
// ==========================================

bool JsonValue::operator==(const JsonValue& other) const
{
    return mImpl->json == other.mImpl->json;
}

bool JsonValue::operator!=(const JsonValue& other) const
{
    return !(*this == other);
}

} // namespace ucf::utilities
