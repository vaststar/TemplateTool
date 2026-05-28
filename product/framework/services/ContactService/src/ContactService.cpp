#include "ContactService.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>

#include "ContactManager.h"
#include "ContactServiceLogger.h"

namespace ucf::service {

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic///////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

class ContactService::DataPrivate {
public:
    explicit DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ucf::framework::ICoreFrameworkWPtr getCoreFramework() const;
    ContactManager& getManager();
    const ContactManager& getManager() const;
private:
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    std::unique_ptr<ContactManager> mManager;
};

ContactService::DataPrivate::DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
    , mManager(std::make_unique<ContactManager>(coreFramework))
{
}

ucf::framework::ICoreFrameworkWPtr ContactService::DataPrivate::getCoreFramework() const
{
    return mCoreFrameworkWPtr;
}

ContactManager& ContactService::DataPrivate::getManager()
{
    return *mManager;
}

const ContactManager& ContactService::DataPrivate::getManager() const
{
    return *mManager;
}

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start ContactService Logic////////////////////////////////////////
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
        if (auto dataWarehouseService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock())
        {
            dataWarehouseService->registerCallback(shared_from_this());
        }
    }
}

std::string ContactService::getServiceName() const
{
    return "ContactService";
}

void ContactService::onServiceInitialized()
{
    SERVICE_LOG_DEBUG("ContactService initialized");
}

void ContactService::onCoreFrameworkExit()
{
    SERVICE_LOG_DEBUG("ContactService exiting");
}

void ContactService::OnDatabaseInitialized(const std::string& dbId)
{
    mDataPrivate->getManager().onDatabaseReady(dbId);
    fireNotification(&IContactServiceCallback::onContactDirectoryReady);
}

// ===== Read =====

model::PersonContactArray ContactService::getPersonContactList() const
{
    return mDataPrivate->getManager().getPersonContactList();
}

model::GroupContactArray ContactService::getGroupContactList() const
{
    return mDataPrivate->getManager().getGroupContactList();
}

model::ContactRelationArray ContactService::getContactRelations() const
{
    return mDataPrivate->getManager().getContactRelations();
}

model::IPersonContactPtr ContactService::getPersonContact(const std::string& contactId) const
{
    return mDataPrivate->getManager().getPersonContact(contactId);
}

model::IGroupContactPtr ContactService::getGroupContact(const std::string& contactId) const
{
    return mDataPrivate->getManager().getGroupContact(contactId);
}

// ===== Person batch write =====

void ContactService::addPersonContacts(const model::PersonContactArray& persons)
{
    auto accepted = mDataPrivate->getManager().addPersonContacts(persons);
    if (!accepted.empty())
    {
        fireNotification(&IContactServiceCallback::onPersonContactsAdded, accepted);
    }
}

void ContactService::updatePersonContacts(const model::PersonContactArray& persons)
{
    auto accepted = mDataPrivate->getManager().updatePersonContacts(persons);
    if (!accepted.empty())
    {
        fireNotification(&IContactServiceCallback::onPersonContactsUpdated, accepted);
    }
}

void ContactService::removePersonContacts(const std::vector<std::string>& contactIds)
{
    auto accepted = mDataPrivate->getManager().removePersonContacts(contactIds);
    if (!accepted.empty())
    {
        fireNotification(&IContactServiceCallback::onPersonContactsRemoved, accepted);
    }
}

// ===== Group batch write =====

void ContactService::addGroupContacts(const model::GroupContactArray& groups)
{
    auto accepted = mDataPrivate->getManager().addGroupContacts(groups);
    if (!accepted.empty())
    {
        fireNotification(&IContactServiceCallback::onGroupContactsAdded, accepted);
    }
}

void ContactService::updateGroupContacts(const model::GroupContactArray& groups)
{
    auto accepted = mDataPrivate->getManager().updateGroupContacts(groups);
    if (!accepted.empty())
    {
        fireNotification(&IContactServiceCallback::onGroupContactsUpdated, accepted);
    }
}

void ContactService::removeGroupContacts(const std::vector<std::string>& contactIds)
{
    auto accepted = mDataPrivate->getManager().removeGroupContacts(contactIds);
    if (!accepted.empty())
    {
        fireNotification(&IContactServiceCallback::onGroupContactsRemoved, accepted);
    }
}

// ===== Relation batch write =====

void ContactService::addContactRelations(const model::ContactRelationArray& relations)
{
    auto accepted = mDataPrivate->getManager().addContactRelations(relations);
    if (!accepted.empty())
    {
        fireNotification(&IContactServiceCallback::onContactRelationsAdded, accepted);
    }
}

void ContactService::updateContactRelations(const model::ContactRelationArray& relations)
{
    auto accepted = mDataPrivate->getManager().updateContactRelations(relations);
    if (!accepted.empty())
    {
        fireNotification(&IContactServiceCallback::onContactRelationsUpdated, accepted);
    }
}

void ContactService::removeContactRelations(const std::vector<std::string>& childIds)
{
    auto accepted = mDataPrivate->getManager().removeContactRelations(childIds);
    if (!accepted.empty())
    {
        fireNotification(&IContactServiceCallback::onContactRelationsRemoved, accepted);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish ContactService Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

} // namespace ucf::service
