#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <variant>
#include <ucf/Services/ServiceCommonFile/ServiceExport.h>

namespace ucf::service::model{
namespace DBSupportedTypes{
    using STRING = std::string;
    using INT = int;
    using FLOAT = float;
    using BUFFER = std::vector<uint8_t>;
}
using DBDataValue = std::variant<DBSupportedTypes::STRING, DBSupportedTypes::INT, DBSupportedTypes::FLOAT, DBSupportedTypes::BUFFER>;
class SERVICE_EXPORT DataBaseDataValue final
{
public:
    DataBaseDataValue(const std::string& value);
    DataBaseDataValue(const char* value);
    DataBaseDataValue(long value);
    DataBaseDataValue(long long value);
    DataBaseDataValue(int value);
    DataBaseDataValue(float value);
    DataBaseDataValue(bool value);
    DataBaseDataValue(std::vector<uint8_t> buffer);
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

    bool operator>(const DataBaseDataValue& rhs) const;
    bool operator>=(const DataBaseDataValue& rhs) const;
    bool operator<(const DataBaseDataValue& rhs) const;
    bool operator<=(const DataBaseDataValue& rhs) const;
    bool operator==(const DataBaseDataValue& rhs) const;
    bool operator!=(const DataBaseDataValue& rhs) const;
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

using DBDataValues = std::vector<DataBaseDataValue>;
using ListOfDBValues = std::vector<DBDataValues>;
}