#include "ClientInfoManager.h"

#include <ucf/CoreFramework/ICoreFramework.h>

#include <ucf/Services/DataWarehouseService/DataBaseConfig.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>
#include <ucf/Services/DataWarehouseSchema/DataWarehouseSchemas.h>
#include <ucf/Services/DataWarehouseService/DatabaseDataRecord.h>


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
    if (!mIsLanguageReadFromDB)
    {
        if (auto dbService = mCoreFrameworkWPtr.lock()->getService<ucf::service::IDataWarehouseService>().lock())
        {
            dbService->fetchFromDatabase(getSharedDBConfig().getDBId(), db::schema::SettingsTable::TableName, {db::schema::SettingsTable::KeyField, db::schema::SettingsTable::ValField},
                {{db::schema::SettingsTable::KeyField, "Language", ucf::service::model::DBOperatorType::Equal}},[this](const ucf::service::model::DatabaseDataRecords& results){
                    SERVICE_LOG_DEBUG("got language:" << this);
                    for (const auto& res: results)
                    {
                        auto  val = res.getColumnData(db::schema::SettingsTable::ValField);
                        SERVICE_LOG_DEBUG("got language:" <<val.getIntValue());
                    }
                    
            });
        }
    }
    
    return mLanguageType.load();
}

void ClientInfoManager::setApplicationLanguage(model::LanguageType languageType)
{
    mLanguageType.store(languageType);
}

std::vector<model::LanguageType> ClientInfoManager::getSupportedLanguages() const
{
    return {model::LanguageType::ENGLISH, model::LanguageType::CHINESE_SIMPLIFIED, model::LanguageType::CHINESE_TRADITIONAL, model::LanguageType::FRENCH, model::LanguageType::GERMAN, 
            model::LanguageType::ITALIAN, model::LanguageType::SPANISH, model::LanguageType::PORTUGUESE, model::LanguageType::JAPANESE, model::LanguageType::KOREAN, model::LanguageType::RUSSIAN};
}

model::SqliteDBConfig ClientInfoManager::getSharedDBConfig() const
{
    return model::SqliteDBConfig("test", "app_data/shared_db.db");
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start ClientInfoManager Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}