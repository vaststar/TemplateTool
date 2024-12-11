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
    COMMONHEAD_LOG_DEBUG("will init default database");
    ucf::service::model::SqliteDBConfig dbConfig;
    if (auto coreFramework = mCommonHeadFrameworkWptr.lock()->getCoreFramework().lock())
    {
        if (auto clientInfoService = coreFramework->getService<ucf::service::IClientInfoService>().lock())
        {
            if (auto dataWarehouse = coreFramework->getService<ucf::service::IDataWarehouseService>().lock())
            {
                std::vector<ucf::service::model::DBTableModel> tables{ db::schema::UserContactTable{}, db::schema::GroupContactTable{}, db::schema::SettingsTable{} };
                dataWarehouse->initializeDB(std::make_shared<ucf::service::model::SqliteDBConfig>(clientInfoService->getSharedDBConfig().getDBId(), clientInfoService->getSharedDBConfig().getDBFilePath()), tables);
            }
        }
    }
}
}