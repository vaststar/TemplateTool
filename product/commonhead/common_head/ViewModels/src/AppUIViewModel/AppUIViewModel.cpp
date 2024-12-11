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
}