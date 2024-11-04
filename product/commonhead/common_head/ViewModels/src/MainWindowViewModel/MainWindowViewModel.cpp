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

void MainWindowViewModel::initDatabase() const
{
    COMMONHEAD_LOG_DEBUG("initDatabase");
    
    if (auto coreFramework = mCommonHeadFrameworkWptr.lock()->getCoreFramework().lock())
    {
        if (auto dataWarehouse = coreFramework->getService<ucf::service::IDataWarehouseService>().lock())
        {
            std::vector<ucf::service::model::DBTableModel> tables{ db::schema::UserContactTable{}, db::schema::GroupContactTable{} };
            dataWarehouse->initializeDB(std::make_shared<ucf::service::model::SqliteDBConfig>("test", "app_data/shared_db.db"), tables);
            

            ucf::service::model::ListOfDBValues values;
            values.emplace_back(ucf::service::model::DBDataValues{ std::string("test_id"), std::string("test_name"), std::string("243@qq.com") });
            values.emplace_back(ucf::service::model::DBDataValues{ std::string("test_id111"), std::string("test_name11"), std::string("11243@qq.com") });

            dataWarehouse->insertIntoDatabase("test", "UserContact", {"CONTACT_ID", "CONTACT_FULL_NAME", "CONTACT_EMAIL"}, values);

            dataWarehouse->fetchFromDatabase("test", "UserContact", {"CONTACT_ID", "CONTACT_FULL_NAME"}, {{"CONTACT_ID", "test_id", ucf::service::model::DBOperatorType::Not}},[](const ucf::service::model::DatabaseDataRecords& results){
                auto res = results;
                COMMONHEAD_LOG_DEBUG("got data");
            });
        }
    }
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