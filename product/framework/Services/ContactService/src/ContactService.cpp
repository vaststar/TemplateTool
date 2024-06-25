#include "ContactService.h"

#include <sqlite3.h>

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/ServiceCommonFile/ServiceLogger.h>

#include "Adapters/ContactAdapter.h"

namespace ucf::service{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class ContactService::DataPrivate{
public:
    explicit DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ucf::framework::ICoreFrameworkWPtr getCoreFramework() const;
    std::shared_ptr<model::ContactModel> getContactModel() const;
    std::shared_ptr<ucf::adapter::ContactAdapter> getContactAdapter() const;
private:
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    std::shared_ptr<model::ContactModel>  mContactModelPtr;
    std::shared_ptr<ucf::adapter::ContactAdapter> mContactAdapter;
};

ContactService::DataPrivate::DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
    , mContactModelPtr(std::make_shared<model::ContactModel>(coreFramework))
    , mContactAdapter(std::make_shared<ucf::adapter::ContactAdapter>(coreFramework))
{

}

ucf::framework::ICoreFrameworkWPtr ContactService::DataPrivate::getCoreFramework() const
{
    return mCoreFrameworkWPtr;
}

std::shared_ptr<model::ContactModel> ContactService::DataPrivate::getContactModel() const
{
    return mContactModelPtr;
}

std::shared_ptr<ucf::adapter::ContactAdapter> ContactService::DataPrivate::getContactAdapter() const
{
    return mContactAdapter;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start ContactService Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<IContactService> IContactService::CreateInstance(ucf::framework::ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<ContactService>(coreFramework);
}

ContactService::ContactService(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mDataPrivate(std::make_unique<DataPrivate>(coreFramework))
{
    SERVICE_LOG_DEBUG("Create ContactService, address:" << this);
}

ContactService::~ContactService()
{
    SERVICE_LOG_DEBUG("Delete ContactService, address:" << this);
}

void ContactService::initService()
{
    if (auto coreFramework = mDataPrivate->getCoreFramework().lock())
    {
        coreFramework->registerCallback(shared_from_this());
    }

    //
    sqlite3* pDb = NULL;
    sqlite3_initialize();
    sqlite3_open("test_tt.db", &pDb);
}

std::string ContactService::getServiceName() const
{
    return "ContactService";
}

void ContactService::OnServiceInitialized()
{
    SERVICE_LOG_DEBUG("");
}

void ContactService::onCoreFrameworkExit()
{
    SERVICE_LOG_DEBUG("");
}

void ContactService::fetchContactList()
{
    SERVICE_LOG_DEBUG("start fetchContactList");
    std::vector<model::Contact> contactList;
    if (auto contactModel = mDataPrivate->getContactModel())
    {
        contactList = contactModel->getContacts();
    }
    fireNotification(&IContactServiceCallback::OnContactListAvailable, contactList);
    SERVICE_LOG_DEBUG("finish fetchContactList");
}

std::vector<model::Contact> ContactService::getContactList() const
{
    if (auto contactAdapter = mDataPrivate->getContactAdapter())
    {
        contactAdapter->fetchContactInfo("", [](const ucf::service::model::Contact& contact){
            SERVICE_LOG_DEBUG("test fetch contactInfo");
        });
    }
    
    if (auto contactModel = mDataPrivate->getContactModel())
    {
        return contactModel->getContacts();
    }

    return {};
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish ContactService Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}