#include <ucf/Utilities/JsonUtils/JsonParser.h>

#include <nlohmann/json.hpp>

namespace ucf::utilities {

// ==========================================
// PIMPL Implementation
// ==========================================

class JsonParser::Impl
{
public:
    nlohmann::json json;
    bool valid{false};
    std::string error;
};

// ==========================================
// JsonParser
// ==========================================

JsonParser::JsonParser(const std::string& jsonStr)
    : mImpl(std::make_unique<Impl>())
{
    try
    {
        mImpl->json = nlohmann::json::parse(jsonStr);
        mImpl->valid = true;
    }
    catch (const nlohmann::json::parse_error& e)
    {
        mImpl->error = e.what();
        mImpl->valid = false;
    }
}

JsonParser::JsonParser(std::unique_ptr<Impl> impl)
    : mImpl(std::move(impl))
{
}

JsonParser::~JsonParser() = default;

JsonParser::JsonParser(JsonParser&& other) noexcept = default;
JsonParser& JsonParser::operator=(JsonParser&& other) noexcept = default;

// Validation
bool JsonParser::isValid() const
{
    return mImpl->valid;
}

std::string JsonParser::getError() const
{
    return mImpl->error;
}

// Key existence
bool JsonParser::contains(const std::string& key) const
{
    return mImpl->valid && mImpl->json.contains(key);
}

bool JsonParser::isNull(const std::string& key) const
{
    return contains(key) && mImpl->json[key].is_null();
}

bool JsonParser::isObject(const std::string& key) const
{
    return contains(key) && mImpl->json[key].is_object();
}

bool JsonParser::isArray(const std::string& key) const
{
    return contains(key) && mImpl->json[key].is_array();
}

// Primitive getters
std::optional<std::string> JsonParser::getString(const std::string& key) const
{
    if (!contains(key) || !mImpl->json[key].is_string())
    {
        return std::nullopt;
    }
    return mImpl->json[key].get<std::string>();
}

std::optional<int32_t> JsonParser::getInt(const std::string& key) const
{
    if (!contains(key) || !mImpl->json[key].is_number_integer())
    {
        return std::nullopt;
    }
    return mImpl->json[key].get<int32_t>();
}

std::optional<int64_t> JsonParser::getInt64(const std::string& key) const
{
    if (!contains(key) || !mImpl->json[key].is_number_integer())
    {
        return std::nullopt;
    }
    return mImpl->json[key].get<int64_t>();
}

std::optional<uint64_t> JsonParser::getUInt64(const std::string& key) const
{
    if (!contains(key) || !mImpl->json[key].is_number_unsigned())
    {
        return std::nullopt;
    }
    return mImpl->json[key].get<uint64_t>();
}

std::optional<double> JsonParser::getDouble(const std::string& key) const
{
    if (!contains(key) || !mImpl->json[key].is_number())
    {
        return std::nullopt;
    }
    return mImpl->json[key].get<double>();
}

std::optional<bool> JsonParser::getBool(const std::string& key) const
{
    if (!contains(key) || !mImpl->json[key].is_boolean())
    {
        return std::nullopt;
    }
    return mImpl->json[key].get<bool>();
}

// Nested structures
std::optional<JsonParser> JsonParser::getObject(const std::string& key) const
{
    if (!isObject(key))
    {
        return std::nullopt;
    }
    
    auto impl = std::make_unique<Impl>();
    impl->json = mImpl->json[key];
    impl->valid = true;
    return JsonParser(std::move(impl));
}

std::vector<JsonParser> JsonParser::getObjectArray(const std::string& key) const
{
    std::vector<JsonParser> result;
    if (!isArray(key))
    {
        return result;
    }
    
    for (const auto& item : mImpl->json[key])
    {
        if (item.is_object())
        {
            auto impl = std::make_unique<Impl>();
            impl->json = item;
            impl->valid = true;
            result.push_back(JsonParser(std::move(impl)));
        }
    }
    return result;
}

std::vector<std::string> JsonParser::getStringArray(const std::string& key) const
{
    std::vector<std::string> result;
    if (!isArray(key))
    {
        return result;
    }
    
    for (const auto& item : mImpl->json[key])
    {
        if (item.is_string())
        {
            result.push_back(item.get<std::string>());
        }
    }
    return result;
}

std::vector<int64_t> JsonParser::getIntArray(const std::string& key) const
{
    std::vector<int64_t> result;
    if (!isArray(key))
    {
        return result;
    }
    
    for (const auto& item : mImpl->json[key])
    {
        if (item.is_number_integer())
        {
            result.push_back(item.get<int64_t>());
        }
    }
    return result;
}

std::vector<double> JsonParser::getDoubleArray(const std::string& key) const
{
    std::vector<double> result;
    if (!isArray(key))
    {
        return result;
    }
    
    for (const auto& item : mImpl->json[key])
    {
        if (item.is_number())
        {
            result.push_back(item.get<double>());
        }
    }
    return result;
}

// Iteration
std::vector<std::string> JsonParser::getKeys() const
{
    std::vector<std::string> keys;
    if (!mImpl->valid || !mImpl->json.is_object())
    {
        return keys;
    }
    
    for (auto it = mImpl->json.begin(); it != mImpl->json.end(); ++it)
    {
        keys.push_back(it.key());
    }
    return keys;
}

size_t JsonParser::getArraySize(const std::string& key) const
{
    if (!isArray(key))
    {
        return 0;
    }
    return mImpl->json[key].size();
}

} // namespace ucf::utilities
