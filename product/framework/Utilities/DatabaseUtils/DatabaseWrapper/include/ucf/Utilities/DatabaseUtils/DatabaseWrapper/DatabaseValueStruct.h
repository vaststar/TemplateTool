#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <variant>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseWrapperExport.h>

namespace ucf::utilities::database{

namespace DBSupportedTypes{
    using STRING = std::string;
    using INT = int;
    using FLOAT = float;
    using BLOB = std::vector<uint8_t>;
}
using DBDataValue = std::variant<DBSupportedTypes::STRING, DBSupportedTypes::INT, DBSupportedTypes::FLOAT, DBSupportedTypes::BLOB>;

class DATABASEWRAPPER_EXPORT DatabaseValueStruct final
{
public:
    DatabaseValueStruct(const std::string& value);
    DatabaseValueStruct(const char* value);
    DatabaseValueStruct(long value);
    DatabaseValueStruct(long long value);
    DatabaseValueStruct(int value);
    DatabaseValueStruct(float value);
    DatabaseValueStruct(bool value);
    DatabaseValueStruct(std::vector<uint8_t> buffer);
private:
    DBDataValue mVariantValue;
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

    bool operator>(const DatabaseValueStruct& rhs) const;
    bool operator>=(const DatabaseValueStruct& rhs) const;
    bool operator<(const DatabaseValueStruct& rhs) const;
    bool operator<=(const DatabaseValueStruct& rhs) const;
    bool operator==(const DatabaseValueStruct& rhs) const;
    bool operator!=(const DatabaseValueStruct& rhs) const;

    template <typename T>
    T getVariantValue(const T& staticDefault) const
    {
        if (const auto p = std::get_if<T>(&mVariantValue))
        {
            return *p;
        }
        return staticDefault;
    }
};
}