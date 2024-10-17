#include <tuple>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseValueStruct.h>

namespace ucf::utilities::database{
DatabaseValueStruct::DatabaseValueStruct(const std::string& value)
    : mVariantValue(std::move(value))
{
}

DatabaseValueStruct::DatabaseValueStruct(const char* value)
    : mVariantValue(DBSupportedTypes::STRING(value))
{
}

DatabaseValueStruct::DatabaseValueStruct(long value)
    : mVariantValue(static_cast<DBSupportedTypes::INT>(value))
{
}

DatabaseValueStruct::DatabaseValueStruct(long long value)
    : mVariantValue(static_cast<DBSupportedTypes::INT>(value))
{
}

DatabaseValueStruct::DatabaseValueStruct(int value)
    : mVariantValue(value)
{
}

DatabaseValueStruct::DatabaseValueStruct(float value)
    : mVariantValue(value)
{
}

DatabaseValueStruct::DatabaseValueStruct(bool value)
    : mVariantValue(static_cast<DBSupportedTypes::INT>(value))
{
}

DatabaseValueStruct::DatabaseValueStruct(std::vector<uint8_t> buffer)
    : mVariantValue(std::move(buffer))
{
}

DBSupportedTypes::STRING DatabaseValueStruct::getStringValue() const
{
    static const DBSupportedTypes::STRING defaultString = {};
    return getVariantValue<DBSupportedTypes::STRING>(defaultString);
}

DBSupportedTypes::INT DatabaseValueStruct::getIntValue() const
{
    static DBSupportedTypes::INT defaultInt{ 0 };
    return getVariantValue<DBSupportedTypes::INT>(defaultInt);
}

DBSupportedTypes::FLOAT DatabaseValueStruct::getFloatValue() const
{
    static DBSupportedTypes::FLOAT defaultFloat{ 0.0 };
    return getVariantValue<DBSupportedTypes::FLOAT>(defaultFloat);
}

std::vector<uint8_t> DatabaseValueStruct::getBufferValue() const
{
    static const DBSupportedTypes::BLOB defaultBlob{};
    return getVariantValue<DBSupportedTypes::BLOB>(defaultBlob);
}

bool DatabaseValueStruct::operator>(const DatabaseValueStruct& rhs) const
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

bool DatabaseValueStruct::operator>=(const DatabaseValueStruct& rhs) const
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

bool DatabaseValueStruct::operator<(const DatabaseValueStruct& rhs) const
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

bool DatabaseValueStruct::operator<=(const DatabaseValueStruct& rhs) const
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

bool DatabaseValueStruct::operator==(const DatabaseValueStruct& rhs) const
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
    return false;
}

bool DatabaseValueStruct::operator!=(const DatabaseValueStruct& rhs) const
{
    return !operator==(rhs);
}
}