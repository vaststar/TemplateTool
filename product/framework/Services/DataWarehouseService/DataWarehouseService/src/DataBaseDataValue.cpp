#include <tuple>
#include <ucf/Services/DataWarehouseService/DatabaseDataValue.h>

namespace ucf::service::model{
DatabaseDataValue::DatabaseDataValue(const std::string& value)
    : mVariantValue(std::move(value))
{
}

DatabaseDataValue::DatabaseDataValue(const char* value)
    : mVariantValue(DBSupportedTypes::STRING(value))
{
}

DatabaseDataValue::DatabaseDataValue(long value)
    : mVariantValue(static_cast<DBSupportedTypes::INT>(value))
{
}

DatabaseDataValue::DatabaseDataValue(long long value)
    : mVariantValue(static_cast<DBSupportedTypes::INT>(value))
{
}

DatabaseDataValue::DatabaseDataValue(int value)
    : mVariantValue(value)
{
}

DatabaseDataValue::DatabaseDataValue(float value)
    : mVariantValue(value)
{
}

DatabaseDataValue::DatabaseDataValue(bool value)
    : mVariantValue(static_cast<DBSupportedTypes::INT>(value))
{
}

DatabaseDataValue::DatabaseDataValue(std::vector<uint8_t> buffer)
    : mVariantValue(std::move(buffer))
{
}

DBSupportedTypes::STRING DatabaseDataValue::getStringValue() const
{
    static const DBSupportedTypes::STRING defaultString = {};
    return getVariantValue<DBSupportedTypes::STRING>(defaultString);
}

DBSupportedTypes::INT DatabaseDataValue::getIntValue() const
{
    static DBSupportedTypes::INT defaultInt{ 0 };
    return getVariantValue<DBSupportedTypes::INT>(defaultInt);
}

DBSupportedTypes::FLOAT DatabaseDataValue::getFloatValue() const
{
    static DBSupportedTypes::FLOAT defaultFloat{ 0.0 };
    return getVariantValue<DBSupportedTypes::FLOAT>(defaultFloat);
}

DBSupportedTypes::BUFFER DatabaseDataValue::getBufferValue() const
{
    static const DBSupportedTypes::BUFFER defaultBlob{};
    return getVariantValue<DBSupportedTypes::BUFFER>(defaultBlob);
}

bool DatabaseDataValue::operator>(const DatabaseDataValue& rhs) const
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

bool DatabaseDataValue::operator>=(const DatabaseDataValue& rhs) const
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

bool DatabaseDataValue::operator<(const DatabaseDataValue& rhs) const
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

bool DatabaseDataValue::operator<=(const DatabaseDataValue& rhs) const
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

bool DatabaseDataValue::operator==(const DatabaseDataValue& rhs) const
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

bool DatabaseDataValue::operator!=(const DatabaseDataValue& rhs) const
{
    return !operator==(rhs);
}
}