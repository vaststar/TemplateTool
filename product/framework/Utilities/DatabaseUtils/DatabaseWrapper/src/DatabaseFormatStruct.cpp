#include <tuple>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseFormatStruct.h>

namespace ucf::utilities::database{
DBFormatStruct::DBFormatStruct(DBSupportedTypes::STRING value)
    : mVariantValue(std::move(value))
{
}

DBFormatStruct::DBFormatStruct(const char* value)
    : mVariantValue(DBSupportedTypes::STRING(value))
{
}

DBFormatStruct::DBFormatStruct(long value)
    : mVariantValue((DBSupportedTypes::INT)value)
{
}

DBFormatStruct::DBFormatStruct(long long value)
    : mVariantValue((DBSupportedTypes::INT)value)
{
}

DBFormatStruct::DBFormatStruct(int value)
    : mVariantValue((DBSupportedTypes::INT)value)
{
}

DBFormatStruct::DBFormatStruct(float value)
    : mVariantValue(value)
{
}

DBFormatStruct::DBFormatStruct(bool value)
    : mVariantValue((DBSupportedTypes::INT)value)
{
}

DBFormatStruct::DBFormatStruct(DBSupportedTypes::BLOB buffer)
    : mVariantValue(std::move(buffer))
{
}

// DBFormatStruct::DBFormatStruct(DBSupportedTypes::STRING_VECTOR value)
//     : mVariantValue(std::move(value))
// {
// }

// DBFormatStruct::DBFormatStruct(DBSupportedTypes::INT_VECTOR values)
//     : mVariantValue(std::move(values))
// {
// }

DBSupportedTypes::STRING DBFormatStruct::getStringValue() const
{
    static const DBSupportedTypes::STRING defaultString = {};
    return getVariantValue<DBSupportedTypes::STRING>(defaultString);
}

DBSupportedTypes::INT DBFormatStruct::getIntValue() const
{
    static DBSupportedTypes::INT defaultInt{ 0 };
    return getVariantValue<DBSupportedTypes::INT>(defaultInt);
}

DBSupportedTypes::FLOAT DBFormatStruct::getFloatValue() const
{
    static DBSupportedTypes::FLOAT defaultFloat{ 0.0 };
    return getVariantValue<DBSupportedTypes::FLOAT>(defaultFloat);
}

DBSupportedTypes::BLOB DBFormatStruct::getBufferValue() const
{
    static const DBSupportedTypes::BLOB defaultBlob{};
    return getVariantValue<DBSupportedTypes::BLOB>(defaultBlob);
}

// const DBSupportedTypes::STRING_VECTOR& DBFormatStruct::getStringVectorValue() const
// {
//     static const DBSupportedTypes::STRING_VECTOR defaultStringFieldVector{};
//     return getVariantValue<DBSupportedTypes::STRING_VECTOR>(defaultStringFieldVector);
// }

// const DBSupportedTypes::INT_VECTOR& DBFormatStruct::getIntVectorValues() const
// {
//     static const DBSupportedTypes::INT_VECTOR defaultIntVector{};
//     return getVariantValue<DBSupportedTypes::INT_VECTOR>(defaultIntVector);
// }

// const DBSupportedTypes::INT DBFormatStruct::getIntVectorValue(size_t index) const
// {
//     static const DBSupportedTypes::INT_VECTOR defaultIntVector{};
//     const auto& values =  getVariantValue<DBSupportedTypes::INT_VECTOR>(defaultIntVector);
//     return values.at(index);
// }

// std::tuple<std::string,int> DBFormatStruct::getCommaSeparatedValues(int currentIndex) const
// {
//     const auto getSeparator = [](const auto& container, const auto& it) -> std::string {
//         return std::next(it) == std::end(container) ? "" : ", ";
//     };

//     std::string result;

//     if (holdsType<DBSupportedTypes::INT_VECTOR>())
//     {
//         auto& values = getIntVectorValues();

//         for (auto it = std::begin(values); it != std::end(values); ++it)
//         {
//             result += "?" + std::to_string(currentIndex++) + getSeparator(values, it);
//         }
//         return std::make_tuple(result, currentIndex);
//     }

//     if (holdsType<DBSupportedTypes::STRING_VECTOR>())
//     {
//         auto& values = getStringVectorValue();

//         for (auto it = std::begin(values); it != std::end(values); ++it)
//         {
//             result += "?" + std::to_string(currentIndex++) + getSeparator(values, it);
            
//         }
//         return std::make_tuple(result, currentIndex);
//     }

//     return std::make_tuple("", 0);
// }

// size_t DBFormatStruct::count_params() const
// {
//     if (holdsType<DBSupportedTypes::STRING_VECTOR>())
//     {
//         const auto& v = *std::get_if<DBSupportedTypes::STRING_VECTOR>(&mVariantValue);
//         return v.size();
//     }
//     if (holdsType<DBSupportedTypes::INT_VECTOR>())
//     {
//         const auto& v = *std::get_if<DBSupportedTypes::INT_VECTOR>(&mVariantValue);
//         return v.size();
//     }
//     return 1;
// }

bool DBFormatStruct::operator>(const DBFormatStruct& rhs) const
{
    if (holdsType<DBSupportedTypes::INT>())
    {
        return *std::get_if<DBSupportedTypes::INT>(&mVariantValue) > *std::get_if<DBSupportedTypes::INT>(&rhs.mVariantValue);
    }
    else if (holdsType<DBSupportedTypes::FLOAT>())
    {
        return *std::get_if<DBSupportedTypes::FLOAT>(&mVariantValue) > *std::get_if<DBSupportedTypes::FLOAT>(&rhs.mVariantValue);
    }
    return false;
}

bool DBFormatStruct::operator>=(const DBFormatStruct& rhs) const
{
    if (holdsType<DBSupportedTypes::INT>())
    {
        return *std::get_if<DBSupportedTypes::INT>(&mVariantValue) >= *std::get_if<DBSupportedTypes::INT>(&rhs.mVariantValue);
    }
    else if (holdsType<DBSupportedTypes::FLOAT>())
    {
        return *std::get_if<DBSupportedTypes::FLOAT>(&mVariantValue) >= *std::get_if<DBSupportedTypes::FLOAT>(&rhs.mVariantValue);
    }
    return false;
}

bool DBFormatStruct::operator<(const DBFormatStruct& rhs) const
{
    if (holdsType<DBSupportedTypes::STRING>())
    {
        return *std::get_if<DBSupportedTypes::STRING>(&mVariantValue) < *std::get_if<DBSupportedTypes::STRING>(&rhs.mVariantValue);
    }
    else  if (holdsType<DBSupportedTypes::INT>())
    {
        return *std::get_if<DBSupportedTypes::INT>(&mVariantValue) < *std::get_if<DBSupportedTypes::INT>(&rhs.mVariantValue);
    }
    else if (holdsType<DBSupportedTypes::FLOAT>())
    {
        return *std::get_if<DBSupportedTypes::FLOAT>(&mVariantValue) < *std::get_if<DBSupportedTypes::FLOAT>(&rhs.mVariantValue);
    }
    return false;
}

bool DBFormatStruct::operator<=(const DBFormatStruct& rhs) const
{
    if (holdsType<DBSupportedTypes::INT>())
    {
        return *std::get_if<DBSupportedTypes::INT>(&mVariantValue) <= *std::get_if<DBSupportedTypes::INT>(&rhs.mVariantValue);
    }
    else if (holdsType<DBSupportedTypes::FLOAT>())
    {
        return *std::get_if<DBSupportedTypes::FLOAT>(&mVariantValue) <= *std::get_if<DBSupportedTypes::FLOAT>(&rhs.mVariantValue);
    }
    return false;
}

bool DBFormatStruct::operator==(const DBFormatStruct& rhs) const
{
    if (holdsType<DBSupportedTypes::STRING>())
    {
        return *std::get_if<DBSupportedTypes::STRING>(&mVariantValue) == *std::get_if<DBSupportedTypes::STRING>(&rhs.mVariantValue);
    }
    else if (holdsType<DBSupportedTypes::INT>())
    {
        return *std::get_if<DBSupportedTypes::INT>(&mVariantValue) == *std::get_if<DBSupportedTypes::INT>(&rhs.mVariantValue);
    }
    else if (holdsType<DBSupportedTypes::FLOAT>())
    {
        return *std::get_if<DBSupportedTypes::FLOAT>(&mVariantValue) == *std::get_if<DBSupportedTypes::FLOAT>(&rhs.mVariantValue);
    }
    // else if (holdsType<DBSupportedTypes::STRING_VECTOR>())
    // {
    //     return *std::get_if <DBSupportedTypes::STRING_VECTOR>(&mVariantValue) == *std::get_if<DBSupportedTypes::STRING_VECTOR>(&rhs.mVariantValue);
    // }
    // else if (holdsType<DBSupportedTypes::INT_VECTOR>())
    // {
    //     return *std::get_if <DBSupportedTypes::INT_VECTOR>(&mVariantValue) == *std::get_if<DBSupportedTypes::INT_VECTOR>(&rhs.mVariantValue);
    // }
    return false;
}

bool DBFormatStruct::operator!=(const DBFormatStruct& rhs) const
{
    return !operator==(rhs);
}
}