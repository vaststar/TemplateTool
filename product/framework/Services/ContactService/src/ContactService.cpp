#include "ContactService.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/ServiceCommonFile/ServiceLogger.h>

#include "ContactModel.h"
#include "ContactManager.h"

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
    std::shared_ptr<ContactModel> getContactModel() const;
    std::shared_ptr<ContactManager> getContactManager() const;
private:
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    std::shared_ptr<ContactModel>  mContactModelPtr;
    std::shared_ptr<ContactManager> mContactManagerPtr;
};

ContactService::DataPrivate::DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
    , mContactModelPtr(std::make_shared<ContactModel>(coreFramework))
    , mContactManagerPtr(std::make_shared<ContactManager>(coreFramework, mContactModelPtr))
{

}

ucf::framework::ICoreFrameworkWPtr ContactService::DataPrivate::getCoreFramework() const
{
    return mCoreFrameworkWPtr;
}

std::shared_ptr<ContactModel> ContactService::DataPrivate::getContactModel() const
{
    return mContactModelPtr;
}

std::shared_ptr<ContactManager> ContactService::DataPrivate::getContactManager() const
{
    return mContactManagerPtr;
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

    mDataPrivate->getContactModel()->initDataBase();
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

void ContactService::fetchContactList(std::function<void(const std::vector<model::Contact>&)> contactListCallback)
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
    return mDataPrivate->getContactManager()->getContactList();
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish ContactService Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}