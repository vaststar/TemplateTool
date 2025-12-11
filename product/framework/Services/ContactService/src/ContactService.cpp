#include "ContactService.h"

#include <ucf/CoreFramework/ICoreFramework.h>

#include "ContactServiceLogger.h"
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

    ContactManager& getContactManager();
    const ContactManager& getContactManager() const;
private:
    const ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    const std::unique_ptr<ContactManager> mContactManagerPtr;
};

ContactService::DataPrivate::DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
    , mContactManagerPtr(std::make_unique<ContactManager>(coreFramework))
{

}

ucf::framework::ICoreFrameworkWPtr ContactService::DataPrivate::getCoreFramework() const
{
    return mCoreFrameworkWPtr;
}

ContactManager& ContactService::DataPrivate::getContactManager()
{
    return *mContactManagerPtr;
}

const ContactManager& ContactService::DataPrivate::getContactManager() const
{
    return *mContactManagerPtr;
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
std::shared_ptr<IContactService> IContactService::createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework)
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
}

std::string ContactService::getServiceName() const
{
    return "ContactService";
}

void ContactService::onServiceInitialized()
{
    SERVICE_LOG_DEBUG("");
}

void ContactService::onCoreFrameworkExit()
{
    SERVICE_LOG_DEBUG("");
}

// void ContactService::fetchContactList(std::function<void(const std::vector<model::Contact>&)> contactListCallback)
// {
//     SERVICE_LOG_DEBUG("start fetchContactList");
//     std::vector<model::Contact> contactList;
//     if (auto contactModel = mDataPrivate->getContactModel())
//     {
//         contactList = contactModel->getContacts();
//     }
//     fireNotification(&IContactServiceCallback::OnContactListAvailable, contactList);
//     SERVICE_LOG_DEBUG("finish fetchContactList");
// }

std::vector<model::IPersonContactPtr> ContactService::getPersonContactList() const
{
    return mDataPrivate->getContactManager().getPersonContactList();
}

model::IPersonContactPtr ContactService::getPersonContact(const std::string& contactId) const
{
    return mDataPrivate->getContactManager().getPersonContact(contactId);
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish ContactService Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}