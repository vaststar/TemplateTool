#include "ClientInfoManager.h"

#include <appVersion.h>
#include <ucf/CoreFramework/ICoreFramework.h>

#include <ucf/Services/DataWarehouseService/DataBaseConfig.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>
#include <ucf/Services/DataWarehouseSchema/DataWarehouseSchemas.h>
#include <ucf/Services/DataWarehouseService/DatabaseDataRecord.h>


#include <ucf/Utilities/UUIDUtils/UUIDUtils.h>
#include <ucf/Utilities/FilePathUtils/FilePathUtils.h>
#include "ClientInfoServiceLogger.h"

namespace ucf::service{
static constexpr const char* APP_INTERNAL_NAME = "TemplateToolApp";
static constexpr const char* APP_INTERNAL_NAME_DEBUG = "TemplateToolAppDebug";
static constexpr const char* APP_DATA_FOLDER_NAME = "app_data";
static constexpr const char* APP_LOG_FOLDER_NAME = "app_log";
static constexpr const char* APP_CRASH_FOLDER_NAME = "app_crash";

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start ClientInfoManager Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
ClientInfoManager::ClientInfoManager(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
    , mLanguageType(model::LanguageType::CHINESE_SIMPLIFIED)
    , mThemeType(model::ThemeType::SystemDefault)
    , mIsLanguageReadFromDB (false)
{
    SERVICE_LOG_DEBUG("Create ClientInfoManager, address:" << this);
    if (!ucf::utilities::FilePathUtils::EnsureDirectoryExists(getDataStoragePath()))
    {
        SERVICE_LOG_DEBUG("Create data storage directory failed, path:" << getDataStoragePath());
    }
    if (!ucf::utilities::FilePathUtils::EnsureDirectoryExists(getLogStoragePath()))
    {
        SERVICE_LOG_DEBUG("Create log storage directory failed, path:" << getLogStoragePath());
    }
}

ClientInfoManager::~ClientInfoManager()
{

}

model::Version ClientInfoManager::getApplicationVersion() const
{
    return model::Version{AppVersion::VERSION_MAJOR, AppVersion::VERSION_MINOR, AppVersion::VERSION_PATCH, AppVersion::VERSION_BUILD};
}

model::ProductInfo ClientInfoManager::getProductInfo() const
{
    return model::ProductInfo{
        AppInfo::COMPANY_NAME,
        AppInfo::COPYRIGHT,
        AppInfo::PRODUCT_NAME,
        AppInfo::PRODUCT_DESCRIPTION
    };
}

model::LanguageType ClientInfoManager::getApplicationLanguage() const
{
    return mLanguageType.load();
}

void ClientInfoManager::setApplicationLanguage(model::LanguageType languageType)
{
    mLanguageType.store(languageType);
    if (auto dbService = mCoreFrameworkWPtr.lock()->getService<ucf::service::IDataWarehouseService>().lock())
    {
        ucf::service::model::ListOfDBValues dbVals;
        dbVals.emplace_back(ucf::service::model::DBDataValues{std::string("Language"), static_cast<int>(languageType)});
        dbService->insertIntoDatabase(getSharedDBConfig().getDBId(), db::schema::SettingsTable::TableName, 
            {db::schema::SettingsTable::KeyField, db::schema::SettingsTable::ValField}, dbVals);
    }
}

std::vector<model::LanguageType> ClientInfoManager::getSupportedLanguages() const
{
    return {model::LanguageType::ENGLISH, model::LanguageType::CHINESE_SIMPLIFIED, model::LanguageType::CHINESE_TRADITIONAL, model::LanguageType::FRENCH, model::LanguageType::GERMAN, 
            model::LanguageType::ITALIAN, model::LanguageType::SPANISH, model::LanguageType::PORTUGUESE, model::LanguageType::JAPANESE, model::LanguageType::KOREAN, model::LanguageType::RUSSIAN};
}

void ClientInfoManager::setCurrentThemeType(model::ThemeType themeType)
{
    mThemeType.store(themeType);
    if (auto dbService = mCoreFrameworkWPtr.lock()->getService<ucf::service::IDataWarehouseService>().lock())
    {
        ucf::service::model::ListOfDBValues dbVals;
        dbVals.emplace_back(ucf::service::model::DBDataValues{std::string("Theme"), static_cast<int>(themeType)});
        dbService->insertIntoDatabase(getSharedDBConfig().getDBId(), db::schema::SettingsTable::TableName, 
            {db::schema::SettingsTable::KeyField, db::schema::SettingsTable::ValField}, dbVals);
    }
}

model::ThemeType ClientInfoManager::getCurrentThemeType() const
{
    return mThemeType.load();
}

std::vector<model::ThemeType> ClientInfoManager::getSupportedThemeTypes() const
{
    return {model::ThemeType::SystemDefault, model::ThemeType::Dark, model::ThemeType::Light};
}

model::SqliteDBConfig ClientInfoManager::getSharedDBConfig() const
{
    return model::SqliteDBConfig("test", ucf::utilities::FilePathUtils::joinPaths(std::filesystem::path(getDataStoragePath()), "shared_database.db").string(), "");
}

void ClientInfoManager::databaseInitialized(const std::string& dbId)
{
    if (auto dbService = mCoreFrameworkWPtr.lock()->getService<ucf::service::IDataWarehouseService>().lock())
    {
        dbService->fetchFromDatabase(getSharedDBConfig().getDBId(), db::schema::SettingsTable::TableName, {db::schema::SettingsTable::KeyField, db::schema::SettingsTable::ValField},
            {{db::schema::SettingsTable::KeyField, "Language", ucf::service::model::DBOperatorType::Equal}},[this](const ucf::service::model::DatabaseDataRecords& results){
                if (results.empty())
                {
                    SERVICE_LOG_DEBUG("no language setting found in database, use default");
                    return;
                }

                for (const auto& res: results)
                {
                    auto  val = res.getColumnData(db::schema::SettingsTable::ValField);
                    mLanguageType.store(static_cast<model::LanguageType>(val.getIntValue()));
                    SERVICE_LOG_DEBUG("got language from database:" << val.getIntValue());
                }
        });
        
        dbService->fetchFromDatabase(getSharedDBConfig().getDBId(), db::schema::SettingsTable::TableName, {db::schema::SettingsTable::KeyField, db::schema::SettingsTable::ValField},
            {{db::schema::SettingsTable::KeyField, "Theme", ucf::service::model::DBOperatorType::Equal}},[this](const ucf::service::model::DatabaseDataRecords& results){
                if (results.empty())
                {
                    SERVICE_LOG_DEBUG("no theme setting found in database, use default");
                    return;
                }

                for (const auto& res: results)
                {
                    auto  val = res.getColumnData(db::schema::SettingsTable::ValField);
                    mThemeType.store(static_cast<model::ThemeType>(val.getIntValue()));
                    SERVICE_LOG_DEBUG("got theme from database:" << val.getIntValue());
                }
        });
    }
}

std::string ClientInfoManager::getDataStoragePath() const
{
#if defined(_DEBUG) || !defined(NDEBUG)
    return ucf::utilities::FilePathUtils::joinPaths(
        ucf::utilities::FilePathUtils::getBaseStorageDir(),
        APP_INTERNAL_NAME_DEBUG,
        APP_DATA_FOLDER_NAME
    ).string();
#else
    return ucf::utilities::FilePathUtils::joinPaths(
        ucf::utilities::FilePathUtils::getBaseStorageDir(),
        APP_INTERNAL_NAME,
        APP_DATA_FOLDER_NAME
    ).string();
#endif
    return {};
}

std::string ClientInfoManager::getLogStoragePath() const
{
#if defined(_DEBUG) || !defined(NDEBUG)
    return ucf::utilities::FilePathUtils::joinPaths(
        ucf::utilities::FilePathUtils::getBaseStorageDir(),
        APP_INTERNAL_NAME_DEBUG,
        APP_LOG_FOLDER_NAME
    ).string();
#else
    return ucf::utilities::FilePathUtils::joinPaths(
        ucf::utilities::FilePathUtils::getBaseStorageDir(),
        APP_INTERNAL_NAME,
        APP_LOG_FOLDER_NAME
    ).string();
#endif
    return {};
}

std::string ClientInfoManager::getCrashStoragePath() const
{
#if defined(_DEBUG) || !defined(NDEBUG)
    return ucf::utilities::FilePathUtils::joinPaths(
        ucf::utilities::FilePathUtils::getBaseStorageDir(),
        APP_INTERNAL_NAME_DEBUG,
        APP_CRASH_FOLDER_NAME
    ).string();
#else
    return ucf::utilities::FilePathUtils::joinPaths(
        ucf::utilities::FilePathUtils::getBaseStorageDir(),
        APP_INTERNAL_NAME,
        APP_CRASH_FOLDER_NAME
    ).string();
#endif
    return {};
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start ClientInfoManager Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}