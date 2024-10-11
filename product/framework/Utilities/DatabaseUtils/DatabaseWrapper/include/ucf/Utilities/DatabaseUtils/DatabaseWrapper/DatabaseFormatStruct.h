#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <variant>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseWrapperExport.h>

namespace ucf::utilities::database{

namespace DBSupportedTypes
{
    using STRING = std::string;
    using INT = int;
    using BLOB = std::vector<uint8_t>;
    using FLOAT = float;
    // using STRING_VECTOR = std::vector<std::string>;
    // using INT_VECTOR = std::vector<INT>;
} // namespace DBSupportedTypes
using DBType = std::variant<DBSupportedTypes::STRING, DBSupportedTypes::INT, DBSupportedTypes::FLOAT, DBSupportedTypes::BLOB/*, DBSupportedTypes::STRING_VECTOR, DBSupportedTypes::INT_VECTOR*/>;


class DATABASEWRAPPER_EXPORT DBFormatStruct final
{
public:
    DBFormatStruct(DBSupportedTypes::STRING value);
    DBFormatStruct(const char* value);
    DBFormatStruct(long value);
    DBFormatStruct(long long value);
    DBFormatStruct(int value);
    DBFormatStruct(float value);
    DBFormatStruct(bool value);
    DBFormatStruct(DBSupportedTypes::BLOB buffer);
    // DBFormatStruct(DBSupportedTypes::STRING_VECTOR value);
    // DBFormatStruct(DBSupportedTypes::INT_VECTOR values);
private:
    // this is the main thing: we store the internal data as a variant over several types
    // the total memory consumption is the biggest of these types which is StringVector (24 bytes) + the std::variant core size (8 bytes or so)
    DBType mVariantValue;
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
    DBSupportedTypes::STRING getStringValue() const;
    DBSupportedTypes::INT getIntValue() const;
    DBSupportedTypes::FLOAT getFloatValue() const;
    DBSupportedTypes::BLOB getBufferValue() const;
    // const DBSupportedTypes::STRING_VECTOR& getStringVectorValue() const;
    // const DBSupportedTypes::INT_VECTOR& getIntVectorValues() const;
    // const DBSupportedTypes::INT getIntVectorValue(size_t index) const;
    // std::tuple<std::string,int> getCommaSeparatedValues(int currentIndex) const;
    // size_t count_params() const;
    bool operator>(const DBFormatStruct& rhs) const;
    bool operator>=(const DBFormatStruct& rhs) const;
    bool operator<(const DBFormatStruct& rhs) const;
    bool operator<=(const DBFormatStruct& rhs) const;
    bool operator==(const DBFormatStruct& rhs) const;
    bool operator!=(const DBFormatStruct& rhs) const;
};
}