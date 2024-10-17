#include <tuple>
#include <ucf/Services/DataWarehouseService/DataBaseDataValue.h>

namespace ucf::service::model{
DataBaseDataValue::DataBaseDataValue(const std::string& value)
    : mVariantValue(std::move(value))
{
}

DataBaseDataValue::DataBaseDataValue(const char* value)
    : mVariantValue(STRING(value))
{
}

DataBaseDataValue::DataBaseDataValue(long value)
    : mVariantValue(static_cast<INT>(value))
{
}

DataBaseDataValue::DataBaseDataValue(long long value)
    : mVariantValue(static_cast<INT>(value))
{
}

DataBaseDataValue::DataBaseDataValue(int value)
    : mVariantValue(static_cast<INT>(value))
{
}

DataBaseDataValue::DataBaseDataValue(float value)
    : mVariantValue(value)
{
}

DataBaseDataValue::DataBaseDataValue(bool value)
    : mVariantValue(static_cast<INT>(value))
{
}

DataBaseDataValue::DataBaseDataValue(std::vector<uint8_t> buffer)
    : mVariantValue(std::move(buffer))
{
}

std::string DataBaseDataValue::getStringValue() const
{
    static const STRING defaultString = {};
    return getVariantValue<STRING>(defaultString);
}

int DataBaseDataValue::getIntValue() const
{
    static INT defaultInt{ 0 };
    return getVariantValue<INT>(defaultInt);
}

float DataBaseDataValue::getFloatValue() const
{
    static FLOAT defaultFloat{ 0.0 };
    return getVariantValue<FLOAT>(defaultFloat);
}

std::vector<uint8_t> DataBaseDataValue::getBufferValue() const
{
    static const BUFFER defaultBlob{};
    return getVariantValue<BUFFER>(defaultBlob);
}

bool DataBaseDataValue::operator>(const DataBaseDataValue& rhs) const
{
    if (holdsType<INT>())
    {
        return *std::get_if<INT>(&mVariantValue) > *std::get_if<INT>(&rhs.mVariantValue);
    }
    else if (holdsType<FLOAT>())
    {
        return *std::get_if<FLOAT>(&mVariantValue) > *std::get_if<FLOAT>(&rhs.mVariantValue);
    }
    return false;
}

bool DataBaseDataValue::operator>=(const DataBaseDataValue& rhs) const
{
    if (holdsType<INT>())
    {
        return *std::get_if<INT>(&mVariantValue) >= *std::get_if<INT>(&rhs.mVariantValue);
    }
    else if (holdsType<FLOAT>())
    {
        return *std::get_if<FLOAT>(&mVariantValue) >= *std::get_if<FLOAT>(&rhs.mVariantValue);
    }
    return false;
}

bool DataBaseDataValue::operator<(const DataBaseDataValue& rhs) const
{
    if (holdsType<STRING>())
    {
        return *std::get_if<STRING>(&mVariantValue) < *std::get_if<STRING>(&rhs.mVariantValue);
    }
    else  if (holdsType<INT>())
    {
        return *std::get_if<INT>(&mVariantValue) < *std::get_if<INT>(&rhs.mVariantValue);
    }
    else if (holdsType<FLOAT>())
    {
        return *std::get_if<FLOAT>(&mVariantValue) < *std::get_if<FLOAT>(&rhs.mVariantValue);
    }
    return false;
}

bool DataBaseDataValue::operator<=(const DataBaseDataValue& rhs) const
{
    if (holdsType<INT>())
    {
        return *std::get_if<INT>(&mVariantValue) <= *std::get_if<INT>(&rhs.mVariantValue);
    }
    else if (holdsType<FLOAT>())
    {
        return *std::get_if<FLOAT>(&mVariantValue) <= *std::get_if<FLOAT>(&rhs.mVariantValue);
    }
    return false;
}

bool DataBaseDataValue::operator==(const DataBaseDataValue& rhs) const
{
    if (holdsType<STRING>())
    {
        return *std::get_if<STRING>(&mVariantValue) == *std::get_if<STRING>(&rhs.mVariantValue);
    }
    else if (holdsType<INT>())
    {
        return *std::get_if<INT>(&mVariantValue) == *std::get_if<INT>(&rhs.mVariantValue);
    }
    else if (holdsType<FLOAT>())
    {
        return *std::get_if<FLOAT>(&mVariantValue) == *std::get_if<FLOAT>(&rhs.mVariantValue);
    }
    return false;
}

bool DataBaseDataValue::operator!=(const DataBaseDataValue& rhs) const
{
    return !operator==(rhs);
}
}