#include "ClientInfoManager.h"

#include <ucf/CoreFramework/ICoreFramework.h>

#include <ucf/Services/DataWarehouseService/DataBaseConfig.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>
#include <ucf/Services/DataWarehouseSchema/DataWarehouseSchemas.h>
#include <ucf/Services/DataWarehouseService/DatabaseDataRecord.h>


#include <ucf/Utilities/UUIDUtils/UUIDUtils.h>
#include "ClientInfoServiceLogger.h"

namespace ucf::service{
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

}

ClientInfoManager::~ClientInfoManager()
{

}

model::Version ClientInfoManager::getApplicationVersion() const
{
    return model::Version{"1", "0", "1"};
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
    return model::SqliteDBConfig("test", "app_data/shared_db.db");
}

void ClientInfoManager::databaseInitialized(const std::string& dbId)
{
    if (auto dbService = mCoreFrameworkWPtr.lock()->getService<ucf::service::IDataWarehouseService>().lock())
    {
        
        //for test
        // ucf::service::model::ListOfDBValues values;
        // values.emplace_back(ucf::service::model::DBDataValues{ std::string("test_id"), std::string("test_name"), std::string("243@qq.com") });
        // values.emplace_back(ucf::service::model::DBDataValues{ std::string("test_id111"), std::string("test_name11"), std::string("11243@qq.com") });

        // dbService->insertIntoDatabase("test", "UserContact", {"CONTACT_ID", "CONTACT_FULL_NAME", "CONTACT_EMAIL"}, values);

        // ucf::service::model::ListOfDBValues values_test;
        // values_test.emplace_back(ucf::service::model::DBDataValues{ std::string("Language"), 5});
        // dbService->insertIntoDatabase("test", "Settings", {"Key", "Value"}, values_test);

        dbService->fetchFromDatabase(getSharedDBConfig().getDBId(), db::schema::SettingsTable::TableName, {db::schema::SettingsTable::KeyField, db::schema::SettingsTable::ValField},
            {{db::schema::SettingsTable::KeyField, "Language", ucf::service::model::DBOperatorType::Equal}},[this](const ucf::service::model::DatabaseDataRecords& results){
                for (const auto& res: results)
                {
                    auto  val = res.getColumnData(db::schema::SettingsTable::ValField);
                    mLanguageType.store(static_cast<model::LanguageType>(val.getIntValue()));
                    SERVICE_LOG_DEBUG("got language from database:" << val.getIntValue());
                }
        });
        
        dbService->fetchFromDatabase(getSharedDBConfig().getDBId(), db::schema::SettingsTable::TableName, {db::schema::SettingsTable::KeyField, db::schema::SettingsTable::ValField},
            {{db::schema::SettingsTable::KeyField, "Theme", ucf::service::model::DBOperatorType::Equal}},[this](const ucf::service::model::DatabaseDataRecords& results){
                for (const auto& res: results)
                {
                    auto  val = res.getColumnData(db::schema::SettingsTable::ValField);
                    mThemeType.store(static_cast<model::ThemeType>(val.getIntValue()));
                    SERVICE_LOG_DEBUG("got theme from database:" << val.getIntValue());
                }
        });
    }
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start ClientInfoManager Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}