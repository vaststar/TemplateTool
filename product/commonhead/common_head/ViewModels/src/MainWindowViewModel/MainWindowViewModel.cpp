#include "MainWindowViewModel.h"

#include <ucf/CoreFramework/ICoreFramework.h>

#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>
#include <ucf/Services/MediaService/IMediaService.h>
#include <ucf/Services/DataWarehouseService/DataBaseConfig.h>
#include <ucf/Services/DataWarehouseService/DataBaseDataValue.h>
#include <ucf/Services/DataWarehouseService/DatabaseDataRecord.h>
#include <ucf/Services/DataWarehouseSchema/DataWarehouseSchemas.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>

namespace commonHead::viewModels{
std::shared_ptr<IMainWindowViewModel> IMainWindowViewModel::createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<MainWindowViewModel>(commonHeadFramework);
}

MainWindowViewModel::MainWindowViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : mCommonHeadFrameworkWptr(commonHeadFramework)
{
    COMMONHEAD_LOG_DEBUG("create MainWindowViewModel");
}

std::string MainWindowViewModel::getViewModelName() const
{
    return "MainWindowViewModel";
}

void MainWindowViewModel::openCamera()
{
    if (auto coreFramework = mCommonHeadFrameworkWptr.lock()->getCoreFramework().lock())
    {
        if (auto media = coreFramework->getService<ucf::service::IMediaService>().lock())
        {
            media->openCamera();
        }
    }
}
}