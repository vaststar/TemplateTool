#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <variant>
#include <cstdint>
#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

namespace ucf::service::model{
namespace DBSupportedTypes{
    using STRING = std::string;
    using INT = int;
    using FLOAT = float;
    using BUFFER = std::vector<uint8_t>;
}
using DBDataValue = std::variant<DBSupportedTypes::STRING, DBSupportedTypes::INT, DBSupportedTypes::FLOAT, DBSupportedTypes::BUFFER>;

/**
 * @brief Wrapper for database column values supporting multiple types.
 */
class SERVICE_EXPORT DatabaseDataValue final
{
public:
    DatabaseDataValue(const std::string& value);
    DatabaseDataValue(const char* value);
    DatabaseDataValue(long value);
    DatabaseDataValue(long long value);
    DatabaseDataValue(int value);
    DatabaseDataValue(float value);
    DatabaseDataValue(bool value);
    DatabaseDataValue(std::vector<uint8_t> buffer);
public:
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
    DBDataValue mVariantValue;
};

using DBDataValues = std::vector<DatabaseDataValue>;
using ListOfDBValues = std::vector<DBDataValues>;
}