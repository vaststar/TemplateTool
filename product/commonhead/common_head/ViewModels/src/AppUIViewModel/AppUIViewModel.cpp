#include "AppUIViewModel.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/ClientInfoService/ClientInfoModel.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>

#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>
#include <ucf/Services/DataWarehouseService/DataBaseConfig.h>
#include <ucf/Services/DataWarehouseService/DataBaseDataValue.h>
#include <ucf/Services/DataWarehouseService/DatabaseDataRecord.h>
#include <ucf/Services/DataWarehouseSchema/DataWarehouseSchemas.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>


namespace commonHead::viewModels{
std::shared_ptr<IAppUIViewModel> IAppUIViewModel::createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<AppUIViewModel>(commonHeadFramework);
}

AppUIViewModel::AppUIViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : mCommonHeadFrameworkWptr(commonHeadFramework)
{
    COMMONHEAD_LOG_DEBUG("create AppUIViewModel");
}

std::string AppUIViewModel::getViewModelName() const
{
    return "AppUIViewModel";
}

void AppUIViewModel::initDatabase()
{
    COMMONHEAD_LOG_DEBUG("initDatabase");
    
    ucf::service::model::SqliteDBConfig dbConfig;
    if (auto coreFramework = mCommonHeadFrameworkWptr.lock()->getCoreFramework().lock())
    {
        if (auto service = coreFramework->getService<ucf::service::IClientInfoService>().lock())
        {
            dbConfig = service->getSharedDBConfig();
        }
    }

    if (auto coreFramework = mCommonHeadFrameworkWptr.lock()->getCoreFramework().lock())
    {
        if (auto dataWarehouse = coreFramework->getService<ucf::service::IDataWarehouseService>().lock())
        {
            std::vector<ucf::service::model::DBTableModel> tables{ db::schema::UserContactTable{}, db::schema::GroupContactTable{}, db::schema::SettingsTable{} };
            dataWarehouse->initializeDB(std::make_shared<ucf::service::model::SqliteDBConfig>(dbConfig.getDBId(), dbConfig.getDBFilePath()), tables);

            ucf::service::model::ListOfDBValues values;
            values.emplace_back(ucf::service::model::DBDataValues{ std::string("test_id"), std::string("test_name"), std::string("243@qq.com") });
            values.emplace_back(ucf::service::model::DBDataValues{ std::string("test_id111"), std::string("test_name11"), std::string("11243@qq.com") });

            dataWarehouse->insertIntoDatabase("test", "UserContact", {"CONTACT_ID", "CONTACT_FULL_NAME", "CONTACT_EMAIL"}, values);

            ucf::service::model::ListOfDBValues values_test;
            values_test.emplace_back(ucf::service::model::DBDataValues{ std::string("test__settings_id"), std::string("Language"), 5});
            dataWarehouse->insertIntoDatabase("test", "Settings", {"SETTINGS_ID", "Key", "Value"}, values_test);
            

        }
    }

}
}