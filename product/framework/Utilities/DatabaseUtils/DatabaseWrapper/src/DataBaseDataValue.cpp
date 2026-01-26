#include <tuple>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseDataValue.h>

namespace ucf::utilities::database{
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

DatabaseDataValue::DatabaseDataValue(DBSupportedTypes::NULL_TYPE)
    : mVariantValue(DBSupportedTypes::NULL_TYPE{})
{
}

bool DatabaseDataValue::isNull() const
{
    return std::holds_alternative<DBSupportedTypes::NULL_TYPE>(mVariantValue);
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

DBSupportedTypes::BLOB DatabaseDataValue::getBufferValue() const
{
    static const DBSupportedTypes::BLOB defaultBlob{};
    return getVariantValue<DBSupportedTypes::BLOB>(defaultBlob);
}

bool DatabaseDataValue::operator>(const DatabaseDataValue& rhs) const
{
    return mVariantValue > rhs.mVariantValue;
}

bool DatabaseDataValue::operator>=(const DatabaseDataValue& rhs) const
{
    return mVariantValue >= rhs.mVariantValue;
}

bool DatabaseDataValue::operator<(const DatabaseDataValue& rhs) const
{
    return mVariantValue < rhs.mVariantValue;
}

bool DatabaseDataValue::operator<=(const DatabaseDataValue& rhs) const
{
    return mVariantValue <= rhs.mVariantValue;
}

bool DatabaseDataValue::operator==(const DatabaseDataValue& rhs) const
{
    return mVariantValue == rhs.mVariantValue;
}

bool DatabaseDataValue::operator!=(const DatabaseDataValue& rhs) const
{
    return mVariantValue != rhs.mVariantValue;
}
}