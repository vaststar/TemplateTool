#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <variant>
#include <ucf/Services/ServiceCommonFile/ServiceExport.h>

namespace ucf::service::model{
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
private:
    using STRING = std::string;
    using INT = int;
    using FLOAT = float;
    using BUFFER = std::vector<uint8_t>;
    using DBDataValue = std::variant<STRING, INT, FLOAT, BUFFER>;
    DBDataValue mVariantValue;
public:
    // this is how consumers can check if the variant holds the type they need
    template <typename T>
    bool holdsType() const
    {
        return std::holds_alternative<T>(mVariantValue);
    }
    // these accessors are for type safety and for to provide utility values to consumers
    template <typename T>
    T getVariantValue(const T& staticDefault) const
    {
        if (const auto p = std::get_if<T>(&mVariantValue))
        {
            return *p;
        }
        return staticDefault;
    }
    std::string getStringValue() const;
    int getIntValue() const;
    float getFloatValue() const;
    std::vector<uint8_t> getBufferValue() const;

    bool operator>(const DataBaseDataValue& rhs) const;
    bool operator>=(const DataBaseDataValue& rhs) const;
    bool operator<(const DataBaseDataValue& rhs) const;
    bool operator<=(const DataBaseDataValue& rhs) const;
    bool operator==(const DataBaseDataValue& rhs) const;
    bool operator!=(const DataBaseDataValue& rhs) const;
};

using DBDataValues = std::vector<DataBaseDataValue>;
using ListOfDBValues = std::vector<DBDataValues>;
}