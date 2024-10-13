#include <map>
#include <mutex>

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/ServiceCommonFile/ServiceLogger.h>

#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>
#include <ucf/Services/DataWarehouseService/DataBaseModel.h>

#include "DataWarehouseSchemas.h"
#include "CommonSettingService.h"

namespace ucf::service{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class CommonSettingService::DataPrivate
{
public:
    explicit DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~DataPrivate();

    void initDatabase();
private:
    ucf::framework::ICoreFrameworkWPtr mCoreFramework;
};

CommonSettingService::DataPrivate::DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFramework(coreFramework)
{

}

CommonSettingService::DataPrivate::~DataPrivate()
{
}

void CommonSettingService::DataPrivate::initDatabase()
{
    if (auto coreFramework = mCoreFramework.lock())
    {
        if (auto dataWarehouse = coreFramework->getService<IDataWarehouseService>().lock())
        {
            std::vector<model::DBTableModel> tables{ db::schema::UserContactTable{}, db::schema::GroupContactTable{} };
            dataWarehouse->initializeDB(std::make_shared<ucf::service::model::SqliteDBConfig>("test", "shared_db.db"), tables);
            

            model::ListOfDBValues values;
            values.emplace_back(model::DBDataValues{ std::string("test_id"), std::string("test_name"), std::string("243@qq.com") });
            values.emplace_back(model::DBDataValues{ std::string("test_id111"), std::string("test_name11"), std::string("11243@qq.com") });

            dataWarehouse->insertIntoDatabase("test", "UserContact", {"CONTACT_ID", "CONTACT_FULL_NAME", "CONTACT_EMAIL"}, values);
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start CommonSettingService Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ICommonSettingService> ICommonSettingService::CreateInstance(ucf::framework::ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<CommonSettingService>(coreFramework);
}

CommonSettingService::CommonSettingService(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mDataPrivate(std::make_unique<CommonSettingService::DataPrivate>(coreFramework))
{
    SERVICE_LOG_INFO("create CommonSettingService");
}

CommonSettingService::~CommonSettingService()
{
    SERVICE_LOG_INFO("delete CommonSettingService");
}

std::string CommonSettingService::getServiceName() const
{
    return "CommonSettingService";
}

void CommonSettingService::initService()
{
    mDataPrivate->initDatabase();
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish CommonSettingService Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}
