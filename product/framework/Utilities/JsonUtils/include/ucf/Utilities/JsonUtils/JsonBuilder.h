#pragma once

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

#include <memory>
#include <string>
#include <vector>
#include <cstdint>

namespace ucf::utilities {

/**
 * @brief RAII JSON builder with fluent API
 * 
 * Provides a type-safe, library-agnostic way to construct JSON documents.
 * Uses PIMPL to hide implementation details (nlohmann::json).
 * 
 * Usage:
 * @code
 *   JsonBuilder builder;
 *   builder.add("name", "test")
 *          .add("count", 42)
 *          .add("enabled", true);
 *   std::string json = builder.build();
 * @endcode
 */
class Utilities_EXPORT JsonBuilder final
{
public:
    JsonBuilder();
    ~JsonBuilder();
    
    // Move only (PIMPL)
    JsonBuilder(JsonBuilder&& other) noexcept;
    JsonBuilder& operator=(JsonBuilder&& other) noexcept;
    JsonBuilder(const JsonBuilder& other);
    JsonBuilder& operator=(const JsonBuilder& other);

    // ==========================================
    // Primitive types
    // ==========================================
    
    JsonBuilder& add(const std::string& key, const std::string& value);
    JsonBuilder& add(const std::string& key, const char* value);
    JsonBuilder& add(const std::string& key, int32_t value);
    JsonBuilder& add(const std::string& key, int64_t value);
    JsonBuilder& add(const std::string& key, uint32_t value);
    JsonBuilder& add(const std::string& key, uint64_t value);
    JsonBuilder& add(const std::string& key, double value);
    JsonBuilder& add(const std::string& key, bool value);
    JsonBuilder& addNull(const std::string& key);

    // ==========================================
    // Nested structures
    // ==========================================
    
    /// Add a nested object
    JsonBuilder& addObject(const std::string& key, const JsonBuilder& obj);
    
    /// Add an array of objects
    JsonBuilder& addArray(const std::string& key, const std::vector<JsonBuilder>& arr);
    
    /// Add an array of strings
    JsonBuilder& addStringArray(const std::string& key, const std::vector<std::string>& arr);
    
    /// Add an array of numbers
    JsonBuilder& addNumberArray(const std::string& key, const std::vector<int64_t>& arr);
    JsonBuilder& addNumberArray(const std::string& key, const std::vector<double>& arr);

    // ==========================================
    // Output
    // ==========================================
    
    /// Build JSON string (compact)
    [[nodiscard]] std::string build() const;
    
    /// Build JSON string with indentation
    [[nodiscard]] std::string buildPretty(int indent = 2) const;

private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
};

} // namespace ucf::utilities
