#include <ucf/Utilities/JsonUtils/JsonBuilder.h>

#include <nlohmann/json.hpp>

namespace ucf::utilities {

// ==========================================
// PIMPL Implementation
// ==========================================

class JsonBuilder::Impl
{
public:
    nlohmann::json json;
};

// ==========================================
// JsonBuilder
// ==========================================

JsonBuilder::JsonBuilder()
    : mImpl(std::make_unique<Impl>())
{
    mImpl->json = nlohmann::json::object();
}

JsonBuilder::~JsonBuilder() = default;

JsonBuilder::JsonBuilder(JsonBuilder&& other) noexcept = default;
JsonBuilder& JsonBuilder::operator=(JsonBuilder&& other) noexcept = default;

JsonBuilder::JsonBuilder(const JsonBuilder& other)
    : mImpl(std::make_unique<Impl>())
{
    mImpl->json = other.mImpl->json;
}

JsonBuilder& JsonBuilder::operator=(const JsonBuilder& other)
{
    if (this != &other)
    {
        mImpl->json = other.mImpl->json;
    }
    return *this;
}

// Primitive types
JsonBuilder& JsonBuilder::add(const std::string& key, const std::string& value)
{
    mImpl->json[key] = value;
    return *this;
}

JsonBuilder& JsonBuilder::add(const std::string& key, const char* value)
{
    mImpl->json[key] = value;
    return *this;
}

JsonBuilder& JsonBuilder::add(const std::string& key, int32_t value)
{
    mImpl->json[key] = value;
    return *this;
}

JsonBuilder& JsonBuilder::add(const std::string& key, int64_t value)
{
    mImpl->json[key] = value;
    return *this;
}

JsonBuilder& JsonBuilder::add(const std::string& key, uint32_t value)
{
    mImpl->json[key] = value;
    return *this;
}

JsonBuilder& JsonBuilder::add(const std::string& key, uint64_t value)
{
    mImpl->json[key] = value;
    return *this;
}

JsonBuilder& JsonBuilder::add(const std::string& key, double value)
{
    mImpl->json[key] = value;
    return *this;
}

JsonBuilder& JsonBuilder::add(const std::string& key, bool value)
{
    mImpl->json[key] = value;
    return *this;
}

JsonBuilder& JsonBuilder::addNull(const std::string& key)
{
    mImpl->json[key] = nullptr;
    return *this;
}

// Nested structures
JsonBuilder& JsonBuilder::addObject(const std::string& key, const JsonBuilder& obj)
{
    mImpl->json[key] = obj.mImpl->json;
    return *this;
}

JsonBuilder& JsonBuilder::addArray(const std::string& key, const std::vector<JsonBuilder>& arr)
{
    mImpl->json[key] = nlohmann::json::array();
    for (const auto& item : arr)
    {
        mImpl->json[key].push_back(item.mImpl->json);
    }
    return *this;
}

JsonBuilder& JsonBuilder::addStringArray(const std::string& key, const std::vector<std::string>& arr)
{
    mImpl->json[key] = arr;
    return *this;
}

JsonBuilder& JsonBuilder::addNumberArray(const std::string& key, const std::vector<int64_t>& arr)
{
    mImpl->json[key] = arr;
    return *this;
}

JsonBuilder& JsonBuilder::addNumberArray(const std::string& key, const std::vector<double>& arr)
{
    mImpl->json[key] = arr;
    return *this;
}

// Output
std::string JsonBuilder::build() const
{
    return mImpl->json.dump();
}

std::string JsonBuilder::buildPretty(int indent) const
{
    return mImpl->json.dump(indent);
}

} // namespace ucf::utilities
