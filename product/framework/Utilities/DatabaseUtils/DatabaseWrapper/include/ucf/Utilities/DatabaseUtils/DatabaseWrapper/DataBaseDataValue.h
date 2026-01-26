#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <variant>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseWrapperExport.h>

namespace ucf::utilities::database{

namespace DBSupportedTypes{
    using STRING = std::string;
    using INT = int64_t;
    using FLOAT = float;
    using BLOB = std::vector<uint8_t>;
    
    /// @brief Represents a SQL NULL value
    struct NULL_TYPE {
        bool operator==(const NULL_TYPE&) const { return true; }
        bool operator!=(const NULL_TYPE&) const { return false; }
        bool operator<(const NULL_TYPE&) const { return false; }
        bool operator<=(const NULL_TYPE&) const { return true; }
        bool operator>(const NULL_TYPE&) const { return false; }
        bool operator>=(const NULL_TYPE&) const { return true; }
    };
}
using DBDataValue = std::variant<DBSupportedTypes::STRING, DBSupportedTypes::INT, DBSupportedTypes::FLOAT, DBSupportedTypes::BLOB, DBSupportedTypes::NULL_TYPE>;

/**
 * @brief Wrapper for database column values supporting multiple types.
 */
class DATABASEWRAPPER_EXPORT DatabaseDataValue final
{
public:
    /// @brief Default constructor creates a NULL value
    DatabaseDataValue() = default;
    
    DatabaseDataValue(const std::string& value);
    DatabaseDataValue(const char* value);
    DatabaseDataValue(long value);
    DatabaseDataValue(long long value);
    DatabaseDataValue(int value);
    DatabaseDataValue(float value);
    DatabaseDataValue(bool value);
    DatabaseDataValue(std::vector<uint8_t> buffer);
    DatabaseDataValue(DBSupportedTypes::NULL_TYPE);
    
    /// @brief Check if this value is NULL
    bool isNull() const;
public:                 
    template <typename T>
    bool holdsType() const
    {
        return std::holds_alternative<T>(mVariantValue);
    }
    
    DBSupportedTypes::STRING getStringValue() const;
    DBSupportedTypes::INT getIntValue() const;
    DBSupportedTypes::FLOAT getFloatValue() const;
    DBSupportedTypes::BLOB getBufferValue() const;

    bool operator>(const DatabaseDataValue& rhs) const;
    bool operator>=(const DatabaseDataValue& rhs) const;
    bool operator<(const DatabaseDataValue& rhs) const;
    bool operator<=(const DatabaseDataValue& rhs) const;
    bool operator==(const DatabaseDataValue& rhs) const;
    bool operator!=(const DatabaseDataValue& rhs) const;
private:
    template <typename T>
    T getVariantValue(const T& staticDefault) const
    {
        if (const auto p = std::get_if<T>(&mVariantValue))
        {
            return *p;
        }
        return staticDefault;
    }
private:
    DBDataValue mVariantValue{DBSupportedTypes::NULL_TYPE{}};  // Default to NULL
};
}