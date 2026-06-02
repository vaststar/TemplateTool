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
    SERVICE_LOG_DEBUG("initService");
    // Inject sink before registering DB callback so no early events are lost.
    auto self = shared_from_this();
    mDataPrivate->getManager().setNotificationSink(
        std::static_pointer_cast<IContactNotificationSink>(self));

    if (auto coreFramework = mDataPrivate->getCoreFramework().lock())
    {
        coreFramework->registerCallback(shared_from_this());
        if (auto dataWarehouseService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock())
        {
            dataWarehouseService->registerCallback(shared_from_this());
            SERVICE_LOG_DEBUG("registered DataWarehouseService callback");
        }
        else
        {
            SERVICE_LOG_ERROR("initService: DataWarehouseService unavailable");
        }
    }
    else
    {
        SERVICE_LOG_ERROR("initService: CoreFramework unavailable");
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
    SERVICE_LOG_DEBUG("OnDatabaseInitialized, dbId:" << dbId);
    // Bind only; load is triggered explicitly via loadContactDirectory().
    mDataPrivate->getManager().bindDatabase(dbId);
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

// ===== Batch write =====
// Events fire via the sink after persistence; not here.

void ContactService::addPersonContacts(const model::PersonContactArray& persons)
{
    SERVICE_LOG_DEBUG("addPersonContacts, count:" << persons.size());
    mDataPrivate->getManager().addPersonContacts(persons);
}

void ContactService::updatePersonContacts(const model::PersonContactArray& persons)
{
    SERVICE_LOG_DEBUG("updatePersonContacts, count:" << persons.size());
    mDataPrivate->getManager().updatePersonContacts(persons);
}

void ContactService::removePersonContacts(const std::vector<std::string>& contactIds)
{
    SERVICE_LOG_DEBUG("removePersonContacts, count:" << contactIds.size());
    mDataPrivate->getManager().removePersonContacts(contactIds);
}

void ContactService::addGroupContacts(const model::GroupContactArray& groups)
{
    SERVICE_LOG_DEBUG("addGroupContacts, count:" << groups.size());
    mDataPrivate->getManager().addGroupContacts(groups);
}

void ContactService::updateGroupContacts(const model::GroupContactArray& groups)
{
    SERVICE_LOG_DEBUG("updateGroupContacts, count:" << groups.size());
    mDataPrivate->getManager().updateGroupContacts(groups);
}

void ContactService::removeGroupContacts(const std::vector<std::string>& contactIds)
{
    SERVICE_LOG_DEBUG("removeGroupContacts, count:" << contactIds.size());
    mDataPrivate->getManager().removeGroupContacts(contactIds);
}

void ContactService::addContactRelations(const model::ContactRelationArray& relations)
{
    SERVICE_LOG_DEBUG("addContactRelations, count:" << relations.size());
    mDataPrivate->getManager().addContactRelations(relations);
}

void ContactService::updateContactRelations(const model::ContactRelationArray& relations)
{
    SERVICE_LOG_DEBUG("updateContactRelations, count:" << relations.size());
    mDataPrivate->getManager().updateContactRelations(relations);
}

void ContactService::removeContactRelations(const std::vector<std::string>& childIds)
{
    SERVICE_LOG_DEBUG("removeContactRelations, count:" << childIds.size());
    mDataPrivate->getManager().removeContactRelations(childIds);
}

// ===== Lifecycle =====
void ContactService::loadContactDirectory()
{
    SERVICE_LOG_DEBUG("loadContactDirectory");
    mDataPrivate->getManager().loadContactDirectory();
}

bool ContactService::isContactDirectoryReady() const
{
    return mDataPrivate->getManager().isContactDirectoryReady();
}

// ===== IContactNotificationSink =====
void ContactService::onPersonContactsAdded(const model::PersonContactArray& persons, ContactNotificationSource /*src*/)
{
    SERVICE_LOG_DEBUG("fire onPersonContactsAdded, count:" << persons.size());
    fireNotification(&IContactServiceCallback::onPersonContactsAdded, persons);
}

void ContactService::onPersonContactsUpdated(const model::PersonContactArray& persons, ContactNotificationSource /*src*/)
{
    SERVICE_LOG_DEBUG("fire onPersonContactsUpdated, count:" << persons.size());
    fireNotification(&IContactServiceCallback::onPersonContactsUpdated, persons);
}

void ContactService::onPersonContactsRemoved(const std::vector<std::string>& contactIds, ContactNotificationSource /*src*/)
{
    SERVICE_LOG_DEBUG("fire onPersonContactsRemoved, count:" << contactIds.size());
    fireNotification(&IContactServiceCallback::onPersonContactsRemoved, contactIds);
}

void ContactService::onGroupContactsAdded(const model::GroupContactArray& groups, ContactNotificationSource /*src*/)
{
    SERVICE_LOG_DEBUG("fire onGroupContactsAdded, count:" << groups.size());
    fireNotification(&IContactServiceCallback::onGroupContactsAdded, groups);
}

void ContactService::onGroupContactsUpdated(const model::GroupContactArray& groups, ContactNotificationSource /*src*/)
{
    SERVICE_LOG_DEBUG("fire onGroupContactsUpdated, count:" << groups.size());
    fireNotification(&IContactServiceCallback::onGroupContactsUpdated, groups);
}

void ContactService::onGroupContactsRemoved(const std::vector<std::string>& contactIds, ContactNotificationSource /*src*/)
{
    SERVICE_LOG_DEBUG("fire onGroupContactsRemoved, count:" << contactIds.size());
    fireNotification(&IContactServiceCallback::onGroupContactsRemoved, contactIds);
}

void ContactService::onContactRelationsAdded(const model::ContactRelationArray& relations, ContactNotificationSource /*src*/)
{
    SERVICE_LOG_DEBUG("fire onContactRelationsAdded, count:" << relations.size());
    fireNotification(&IContactServiceCallback::onContactRelationsAdded, relations);
}

void ContactService::onContactRelationsUpdated(const model::ContactRelationArray& relations, ContactNotificationSource /*src*/)
{
    SERVICE_LOG_DEBUG("fire onContactRelationsUpdated, count:" << relations.size());
    fireNotification(&IContactServiceCallback::onContactRelationsUpdated, relations);
}

void ContactService::onContactRelationsRemoved(const std::vector<std::string>& childIds, ContactNotificationSource /*src*/)
{
    SERVICE_LOG_DEBUG("fire onContactRelationsRemoved, count:" << childIds.size());
    fireNotification(&IContactServiceCallback::onContactRelationsRemoved, childIds);
}

void ContactService::onDirectoryLoaded()
{
    SERVICE_LOG_DEBUG("fire onContactDirectoryReady");
    fireNotification(&IContactServiceCallback::onContactDirectoryReady);
}

void ContactService::onDirectoryLoadFailed(ContactDirectoryLoadError error)
{
    SERVICE_LOG_ERROR("fire onContactDirectoryLoadFailed, error:" << static_cast<int>(error));
    fireNotification(&IContactServiceCallback::onContactDirectoryLoadFailed, error);
}

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish ContactService Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

} // namespace ucf::service
