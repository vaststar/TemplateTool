#include <tuple>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DataBaseDataValue.h>

namespace ucf::utilities::database{
DataBaseDataValue::DataBaseDataValue(const std::string& value)
    : mVariantValue(std::move(value))
{
}

DataBaseDataValue::DataBaseDataValue(const char* value)
    : mVariantValue(DBSupportedTypes::STRING(value))
{
}

DataBaseDataValue::DataBaseDataValue(long value)
    : mVariantValue(static_cast<DBSupportedTypes::INT>(value))
{
}

DataBaseDataValue::DataBaseDataValue(long long value)
    : mVariantValue(static_cast<DBSupportedTypes::INT>(value))
{
}

DataBaseDataValue::DataBaseDataValue(int value)
    : mVariantValue(value)
{
}

DataBaseDataValue::DataBaseDataValue(float value)
    : mVariantValue(value)
{
}

DataBaseDataValue::DataBaseDataValue(bool value)
    : mVariantValue(static_cast<DBSupportedTypes::INT>(value))
{
}

DataBaseDataValue::DataBaseDataValue(std::vector<uint8_t> buffer)
    : mVariantValue(std::move(buffer))
{
}

DBSupportedTypes::STRING DataBaseDataValue::getStringValue() const
{
    static const DBSupportedTypes::STRING defaultString = {};
    return getVariantValue<DBSupportedTypes::STRING>(defaultString);
}

DBSupportedTypes::INT DataBaseDataValue::getIntValue() const
{
    static DBSupportedTypes::INT defaultInt{ 0 };
    return getVariantValue<DBSupportedTypes::INT>(defaultInt);
}

DBSupportedTypes::FLOAT DataBaseDataValue::getFloatValue() const
{
    static DBSupportedTypes::FLOAT defaultFloat{ 0.0 };
    return getVariantValue<DBSupportedTypes::FLOAT>(defaultFloat);
}

std::vector<uint8_t> DataBaseDataValue::getBufferValue() const
{
    static const DBSupportedTypes::BLOB defaultBlob{};
    return getVariantValue<DBSupportedTypes::BLOB>(defaultBlob);
}

bool DataBaseDataValue::operator>(const DataBaseDataValue& rhs) const
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

bool DataBaseDataValue::operator>=(const DataBaseDataValue& rhs) const
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

bool DataBaseDataValue::operator<(const DataBaseDataValue& rhs) const
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

bool DataBaseDataValue::operator<=(const DataBaseDataValue& rhs) const
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

bool DataBaseDataValue::operator==(const DataBaseDataValue& rhs) const
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

bool DataBaseDataValue::operator!=(const DataBaseDataValue& rhs) const
{
    return !operator==(rhs);
}
}