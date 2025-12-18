#include "AppUIViewModel.h"

#include <ucf/Services/InvocationService/IInvocationService.h>

#include <ucf/Services/ClientInfoService/ClientInfoModel.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>

#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>
#include <ucf/Services/DataWarehouseService/DataBaseConfig.h>
#include <ucf/Services/DataWarehouseService/DataBaseDataValue.h>
#include <ucf/Services/DataWarehouseService/DatabaseDataRecord.h>
#include <ucf/Services/DataWarehouseSchema/DataWarehouseSchemas.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>
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

void AppUIViewModel::initApplication()
{
    COMMONHEAD_LOG_DEBUG("AppUIViewModel initialized");
    initDatabase();

    fireNotification(&IAppUIViewModelCallback::onShowMainWindow);
}

void AppUIViewModel::initDatabase()
{
    COMMONHEAD_LOG_DEBUG("will init default database");
    if (auto commonHeadFramework = mCommonHeadFrameworkWptr.lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto clientInfoService = serviceLocator->getClientInfoService().lock())
            {
                if (auto dataWarehouseService = serviceLocator->getDataWarehouseService().lock())
                {
                    ucf::service::model::SqliteDBConfig dbConfig = clientInfoService->getSharedDBConfig();
                    COMMONHEAD_LOG_DEBUG("init database with dbId:" << dbConfig.getDBId() << ", dbFilePath:" << dbConfig.getDBFilePath());
                    std::vector<ucf::service::model::DBTableModel> tables{ db::schema::UserContactTable{}, db::schema::GroupContactTable{}, db::schema::SettingsTable{} };
                    dataWarehouseService->initializeDB(std::make_shared<ucf::service::model::SqliteDBConfig>(clientInfoService->getSharedDBConfig().getDBId(), clientInfoService->getSharedDBConfig().getDBFilePath()), tables);
                    fireNotification(&IAppUIViewModelCallback::onDatabaseInitialized);
                }
            }
        }
    }
}

}
