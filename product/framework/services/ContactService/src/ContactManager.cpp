#include "ContactManager.h"

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

model::GroupContactArray ContactManager::getGroupContactList() const
{
    return mContactModel->getGroupContacts();
}

model::ContactRelationArray ContactManager::getContactRelations() const
{
    return mContactModel->getContactRelations();
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

std::vector<std::string> ContactManager::removeContactRelations(const std::vector<std::string>& childIds)
{
    return mContactModel->removeContactRelations(childIds);
}

// ===== Lifecycle =====

void ContactManager::onDatabaseReady(const std::string& databaseId)
{
    mContactModel->onDatabaseReady(databaseId);
}

} // namespace ucf::service

