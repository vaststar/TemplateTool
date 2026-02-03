#pragma once

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <cstdint>

namespace ucf::utilities {

/**
 * @brief JSON parser with type-safe accessors
 * 
 * Provides a library-agnostic way to parse and access JSON values.
 * Uses PIMPL to hide implementation details (nlohmann::json).
 * 
 * Usage:
 * @code
 *   JsonParser parser(jsonString);
 *   if (parser.isValid()) {
 *       std::string name = parser.getString("name").value_or("");
 *       int count = parser.getInt("count").value_or(0);
 *   }
 * @endcode
 */
class Utilities_EXPORT JsonParser final
{
public:
    /// Parse from JSON string
    explicit JsonParser(const std::string& jsonStr);
    ~JsonParser();
    
    // Move only (PIMPL)
    JsonParser(JsonParser&& other) noexcept;
    JsonParser& operator=(JsonParser&& other) noexcept;
    JsonParser(const JsonParser&) = delete;
    JsonParser& operator=(const JsonParser&) = delete;

    // ==========================================
    // Validation
    // ==========================================
    
    /// Check if parsing was successful
    [[nodiscard]] bool isValid() const;
    
    /// Get parse error message (empty if valid)
    [[nodiscard]] std::string getError() const;

    // ==========================================
    // Key existence
    // ==========================================
    
    [[nodiscard]] bool contains(const std::string& key) const;
    [[nodiscard]] bool isNull(const std::string& key) const;
    [[nodiscard]] bool isObject(const std::string& key) const;
    [[nodiscard]] bool isArray(const std::string& key) const;

    // ==========================================
    // Primitive getters (return nullopt if missing or wrong type)
    // ==========================================
    
    [[nodiscard]] std::optional<std::string> getString(const std::string& key) const;
    [[nodiscard]] std::optional<int32_t> getInt(const std::string& key) const;
    [[nodiscard]] std::optional<int64_t> getInt64(const std::string& key) const;
    [[nodiscard]] std::optional<uint64_t> getUInt64(const std::string& key) const;
    [[nodiscard]] std::optional<double> getDouble(const std::string& key) const;
    [[nodiscard]] std::optional<bool> getBool(const std::string& key) const;

    // ==========================================
    // Nested structures
    // ==========================================
    
    /// Get a nested object as new parser
    [[nodiscard]] std::optional<JsonParser> getObject(const std::string& key) const;
    
    /// Get array of objects
    [[nodiscard]] std::vector<JsonParser> getObjectArray(const std::string& key) const;
    
    /// Get array of strings
    [[nodiscard]] std::vector<std::string> getStringArray(const std::string& key) const;
    
    /// Get array of numbers
    [[nodiscard]] std::vector<int64_t> getIntArray(const std::string& key) const;
    [[nodiscard]] std::vector<double> getDoubleArray(const std::string& key) const;

    // ==========================================
    // Iteration (for objects)
    // ==========================================
    
    /// Get all keys in the object
    [[nodiscard]] std::vector<std::string> getKeys() const;
    
    /// Get array size (returns 0 if not an array or if key doesn't exist)
    [[nodiscard]] size_t getArraySize(const std::string& key) const;

private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
    
    // Private constructor for nested objects
    explicit JsonParser(std::unique_ptr<Impl> impl);
};

} // namespace ucf::utilities
