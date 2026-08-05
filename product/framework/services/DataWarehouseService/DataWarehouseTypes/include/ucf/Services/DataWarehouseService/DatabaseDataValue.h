#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <variant>
#include <cstdint>
#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

namespace ucf::service::model{
namespace DBSupportedTypes{
    struct NULL_TYPE {
        constexpr bool operator==(const NULL_TYPE&) const noexcept { return true; }
        constexpr bool operator!=(const NULL_TYPE&) const noexcept { return false; }
    };
    using STRING = std::string;
    using INT = int64_t;
    using FLOAT = float;
    using BUFFER = std::vector<uint8_t>;
}
using DBDataValue = std::variant<DBSupportedTypes::NULL_TYPE, DBSupportedTypes::STRING, DBSupportedTypes::INT, DBSupportedTypes::FLOAT, DBSupportedTypes::BUFFER>;

/**
 * @brief Wrapper for database column values supporting multiple types.
 */
class SERVICE_EXPORT DatabaseDataValue final
{
public:
    /// @brief Default constructor creates a NULL value.
    DatabaseDataValue() = default;
    DatabaseDataValue(const std::string& value);
    DatabaseDataValue(const char* value);
    DatabaseDataValue(long value);
    DatabaseDataValue(long long value);
    DatabaseDataValue(int value);
    DatabaseDataValue(float value);
    DatabaseDataValue(bool value);
    DatabaseDataValue(std::vector<uint8_t> buffer);
public:
    /// @brief Check if value is NULL.
    bool isNull() const;
    
    template <typename T>
    bool holdsType() const
    {
        return std::holds_alternative<T>(mVariantValue);
    }
    DBSupportedTypes::STRING getStringValue() const;
    DBSupportedTypes::INT getIntValue() const;
    DBSupportedTypes::FLOAT getFloatValue() const;
    DBSupportedTypes::BUFFER getBufferValue() const;

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
    DBDataValue mVariantValue{DBSupportedTypes::NULL_TYPE{}};
};

using DBDataValues = std::vector<DatabaseDataValue>;
using ListOfDBValues = std::vector<DBDataValues>;
}