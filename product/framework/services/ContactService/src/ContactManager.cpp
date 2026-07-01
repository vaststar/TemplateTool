#include "ContactManager.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>
#include <ucf/Services/DataWarehouseService/DatabaseConfig.h>

#include "Adapters/ContactAdapter.h"
#include "ContactModel.h"
#include "ContactServiceLogger.h"

namespace ucf::service {

ContactManager::ContactManager(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
    , mContactModel(std::make_unique<ContactModel>(coreFramework))
    , mContactAdapter(std::make_unique<ucf::adapter::ContactAdapter>(coreFramework))
{
    SERVICE_LOG_DEBUG("Create ContactManager, address:" << this);
}

ContactManager::~ContactManager()
{
    SERVICE_LOG_DEBUG("Delete ContactManager, address:" << this);
}

// ===== Read =====

model::PersonContactArray ContactManager::getPersonContactList() const
{
    return mContactModel->getPersonContacts();
}

model::GroupContactArray ContactManager::getGroupContactList(
    std::optional<model::IGroupContact::GroupType> groupType) const
{
    return mContactModel->getGroupContacts(groupType);
}

model::ContactRelationArray ContactManager::getContactRelations(
    std::optional<model::IContactRelation::RelationType> relationType) const
{
    return mContactModel->getContactRelations(relationType);
}

model::IPersonContactPtr ContactManager::getPersonContact(const std::string& contactId) const
{
    return mContactModel->getPersonContact(contactId);
}

model::IGroupContactPtr ContactManager::getGroupContact(const std::string& contactId) const
{
    return mContactModel->getGroupContact(contactId);
}

// ===== Person writes =====

model::PersonContactArray ContactManager::addPersonContacts(const model::PersonContactArray& persons)
{
    return mContactModel->addPersonContacts(persons);
}

model::PersonContactArray ContactManager::updatePersonContacts(const model::PersonContactArray& persons)
{
    return mContactModel->updatePersonContacts(persons);
}

std::vector<std::string> ContactManager::removePersonContacts(const std::vector<std::string>& contactIds)
{
    return mContactModel->removePersonContacts(contactIds);
}

// ===== Group writes =====

model::GroupContactArray ContactManager::addGroupContacts(const model::GroupContactArray& groups)
{
    return mContactModel->addGroupContacts(groups);
}

model::GroupContactArray ContactManager::updateGroupContacts(const model::GroupContactArray& groups)
{
    return mContactModel->updateGroupContacts(groups);
}

std::vector<std::string> ContactManager::removeGroupContacts(const std::vector<std::string>& contactIds)
{
    return mContactModel->removeGroupContacts(contactIds);
}

// ===== Relation writes =====

model::ContactRelationArray ContactManager::addContactRelations(const model::ContactRelationArray& relations)
{
    return mContactModel->addContactRelations(relations);
}

model::ContactRelationArray ContactManager::updateContactRelations(const model::ContactRelationArray& relations)
{
    return mContactModel->updateContactRelations(relations);
}

std::vector<std::string> ContactManager::removeContactRelations(const std::vector<std::string>& relationIds)
{
    return mContactModel->removeContactRelations(relationIds);
}

// ===== Lifecycle =====

void ContactManager::bindDatabase(const std::string& databaseId)
{
    SERVICE_LOG_DEBUG("bindDatabase, databaseId:" << databaseId);
    // Filter: DataWarehouseService may host other DBs; only act on the shared DB
    // owned by ClientInfoService (single source of truth for the shared db id).
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework)
    {
        SERVICE_LOG_ERROR("bindDatabase ignored: CoreFramework unavailable");
        return;
    }
    auto clientInfoService = coreFramework->getService<ucf::service::IClientInfoService>().lock();
    if (!clientInfoService)
    {
        SERVICE_LOG_ERROR("bindDatabase ignored: ClientInfoService unavailable");
        return;
    }
    if (const auto sharedDbId = clientInfoService->getSharedDBConfig().getDBId(); databaseId != sharedDbId)
    {
        SERVICE_LOG_DEBUG("bindDatabase ignored, not shared db, incoming:" << databaseId
                          << ", shared:" << sharedDbId);
        return;
    }
    mContactModel->bindDatabase(databaseId);
    SERVICE_LOG_DEBUG("Loading contact directory after database bind");
    loadContactDirectory();
}

void ContactManager::loadContactDirectory()
{
    SERVICE_LOG_DEBUG("loadContactDirectory");
    mContactModel->loadContactDirectory();
}

bool ContactManager::isContactDirectoryReady() const
{
    return mContactModel->isContactDirectoryReady();
}

void ContactManager::setNotificationSink(std::weak_ptr<IContactNotificationSink> sink)
{
    SERVICE_LOG_DEBUG("setNotificationSink");
    SinkNotifier::setNotificationSink(sink);
    mContactModel->setNotificationSink(std::move(sink));
}

} // namespace ucf::service

