#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseQueryResult.h>

#include <map>
#include <mutex>
#include <algorithm>

#include "DatabaseWrapperLogger.h"
namespace ucf::utilities::database{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class DatabaseQueryResult::DataPrivate
{
public:
    explicit DataPrivate();
    void addColumnData(const std::string& key, const DBFormatStruct& value);
    // DBFormatStruct value(const std::string& key) const;
    std::vector<std::string> keys() const;
    std::map<std::string, DBFormatStruct> values() const;
private:
    std::map<std::string, DBFormatStruct> mValues;
    mutable std::mutex mMutex;
};

DatabaseQueryResult::DataPrivate::DataPrivate()
{

}

void DatabaseQueryResult::DataPrivate::addColumnData(const std::string& key, const DBFormatStruct& value)
{
    if (key.empty())
    {
        return;
    }

    {
        std::scoped_lock<std::mutex> loc(mMutex);
        if (mValues.find(key) != mValues.end())
        {
            DBWRAPPER_LOG_INFO("key already exists, key: " << key << ", will replace it with new value.");
        }
        mValues.emplace(key, value);
    }
}

std::vector<std::string> DatabaseQueryResult::DataPrivate::keys() const
{
    std::vector<std::string> keys;
    {
        std::scoped_lock<std::mutex> loc(mMutex);
        std::transform(mValues.cbegin(), mValues.cend(), std::back_inserter(keys), [](const auto& keyValPair){ return keyValPair.first;});
    }
    return keys;
}

std::map<std::string, DBFormatStruct> DatabaseQueryResult::DataPrivate::values() const
{
    return mValues;
}

// DBFormatStruct DatabaseQueryResult::DataPrivate::value(const std::string& key) const
// {
//     if (key.empty())
//     {
//         return {};
//     }
//     {
//         std::scoped_lock<std::mutex> loc(mMutex);
//         if (auto it = mValues.find(key); it != mValues.end())
//         {
//             return it->second;
//         }
//     }
//     return {};
// }
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

DatabaseQueryResult::DatabaseQueryResult()
    : mDataPrivate(std::make_unique<DatabaseQueryResult::DataPrivate>())
{

}

DatabaseQueryResult::~DatabaseQueryResult()
{

}

void DatabaseQueryResult::addColumnData(const std::string& key, const DBFormatStruct& value)
{
    mDataPrivate->addColumnData(key, value);
}

std::map<std::string, DBFormatStruct> DatabaseQueryResult::values() const
{
    return mDataPrivate->values();
}

// DatabaseValue DatabaseQueryResult::value(const std::string& key) const
// {
//     return mDataPrivate->value(key);
// }

std::vector<std::string> DatabaseQueryResult::keys() const
{
    return mDataPrivate->keys();
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

}