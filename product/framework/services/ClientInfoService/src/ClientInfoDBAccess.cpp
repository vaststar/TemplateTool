#include "ClientInfoDBAccess.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/DataWarehouseSchema/DataWarehouseSchemas.h>
#include <ucf/Services/DataWarehouseService/DatabaseConfig.h>
#include <ucf/Services/DataWarehouseService/DatabaseDataRecord.h>
#include <ucf/Services/DataWarehouseService/DatabaseDataValue.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>

#include "ClientInfoServiceLogger.h"

namespace ucf::service {

namespace {
constexpr const char* kLanguageKey = "Language";
constexpr const char* kThemeKey    = "Theme";
} // namespace

ClientInfoDBAccess::ClientInfoDBAccess(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
{
}

void ClientInfoDBAccess::setDatabaseId(const std::string& dbId)
{
    mDatabaseId = dbId;
}

const std::string& ClientInfoDBAccess::getDatabaseId() const
{
    return mDatabaseId;
}

bool ClientInfoDBAccess::loadLanguage(LoadLanguageCallback callback) const
{
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework || mDatabaseId.empty())
    {
        return false;
    }
    auto dbService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock();
    if (!dbService)
    {
        return false;
    }

    dbService->fetchFromDatabase(
        mDatabaseId,
        db::schema::SettingsTable::TableName,
        {db::schema::SettingsTable::KeyField, db::schema::SettingsTable::ValField},
        {{db::schema::SettingsTable::KeyField, kLanguageKey, ucf::service::model::DBOperatorType::Equal}},
        [cb = std::move(callback)](const ucf::service::model::DatabaseDataRecords& results) {
            std::optional<model::LanguageType> value;
            for (const auto& row : results)
            {
                value = static_cast<model::LanguageType>(
                    row.getColumnData(db::schema::SettingsTable::ValField).getIntValue());
                break;
            }
            SERVICE_LOG_DEBUG("DBAccess loadLanguage, found:" << value.has_value());
            if (cb)
            {
                cb(value);
            }
        });
    return true;
}

bool ClientInfoDBAccess::loadTheme(LoadThemeCallback callback) const
{
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework || mDatabaseId.empty())
    {
        return false;
    }
    auto dbService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock();
    if (!dbService)
    {
        return false;
    }

    dbService->fetchFromDatabase(
        mDatabaseId,
        db::schema::SettingsTable::TableName,
        {db::schema::SettingsTable::KeyField, db::schema::SettingsTable::ValField},
        {{db::schema::SettingsTable::KeyField, kThemeKey, ucf::service::model::DBOperatorType::Equal}},
        [cb = std::move(callback)](const ucf::service::model::DatabaseDataRecords& results) {
            std::optional<model::ThemeType> value;
            for (const auto& row : results)
            {
                value = static_cast<model::ThemeType>(
                    row.getColumnData(db::schema::SettingsTable::ValField).getIntValue());
                break;
            }
            SERVICE_LOG_DEBUG("DBAccess loadTheme, found:" << value.has_value());
            if (cb)
            {
                cb(value);
            }
        });
    return true;
}

void ClientInfoDBAccess::saveLanguage(model::LanguageType languageType) const
{
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework || mDatabaseId.empty())
    {
        return;
    }
    auto dbService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock();
    if (!dbService)
    {
        return;
    }

    ucf::service::model::ListOfDBValues dbVals;
    dbVals.emplace_back(ucf::service::model::DBDataValues{std::string(kLanguageKey), static_cast<int>(languageType)});
    dbService->insertIntoDatabase(
        mDatabaseId,
        db::schema::SettingsTable::TableName,
        {db::schema::SettingsTable::KeyField, db::schema::SettingsTable::ValField},
        dbVals);
}

void ClientInfoDBAccess::saveTheme(model::ThemeType themeType) const
{
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework || mDatabaseId.empty())
    {
        return;
    }
    auto dbService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock();
    if (!dbService)
    {
        return;
    }

    ucf::service::model::ListOfDBValues dbVals;
    dbVals.emplace_back(ucf::service::model::DBDataValues{std::string(kThemeKey), static_cast<int>(themeType)});
    dbService->insertIntoDatabase(
        mDatabaseId,
        db::schema::SettingsTable::TableName,
        {db::schema::SettingsTable::KeyField, db::schema::SettingsTable::ValField},
        dbVals);
}

} // namespace ucf::service
