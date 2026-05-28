#include "ContactModel.h"

#include "ContactDBAccess.h"
#include "ContactServiceLogger.h"

namespace ucf::service {

ContactModel::ContactModel(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mContactDBAccess(std::make_unique<ContactDBAccess>(coreFramework))
{
}

ContactModel::~ContactModel() = default;

// ===== Read =====

model::PersonContactArray ContactModel::getPersonContacts() const
{
    model::PersonContactArray result;
    std::scoped_lock lock(mMutex);
    result.reserve(mPersonContacts.size());
    for (const auto& [_, p] : mPersonContacts)
    {
        result.push_back(p);
    }
    return result;
}

model::GroupContactArray ContactModel::getGroupContacts() const
{
    model::GroupContactArray result;
    std::scoped_lock lock(mMutex);
    result.reserve(mGroupContacts.size());
    for (const auto& [_, g] : mGroupContacts)
    {
        result.push_back(g);
    }
    return result;
}

model::ContactRelationArray ContactModel::getContactRelations() const
{
    model::ContactRelationArray result;
    std::scoped_lock lock(mMutex);
    result.reserve(mContactRelations.size());
    for (const auto& [_, r] : mContactRelations)
    {
        result.push_back(r);
    }
    return result;
}

model::IPersonContactPtr ContactModel::getPersonContact(const std::string& contactId) const
{
    std::scoped_lock lock(mMutex);
    auto it = mPersonContacts.find(contactId);
    if (it == mPersonContacts.end())
    {
        return nullptr;
    }
    return it->second;
}

model::IGroupContactPtr ContactModel::getGroupContact(const std::string& contactId) const
{
    std::scoped_lock lock(mMutex);
    auto it = mGroupContacts.find(contactId);
    if (it == mGroupContacts.end())
    {
        return nullptr;
    }
    return it->second;
}

// ===== Memory primitives =====

model::PersonContactArray ContactModel::addPersonContactsInMemory(const model::PersonContactArray& persons)
{
    model::PersonContactArray accepted;
    accepted.reserve(persons.size());
    std::scoped_lock lock(mMutex);
    for (const auto& p : persons)
    {
        if (!p)
        {
            continue;
        }
        const std::string contactId = p->getContactId();
        if (contactId.empty() || mPersonContacts.find(contactId) != mPersonContacts.end())
        {
            continue;
        }
        auto impl = std::make_shared<model::PersonContact>(contactId);
        impl->setPersonName(p->getPersonName());
        impl->setContactStatus(p->getContactStatus());
        mPersonContacts.emplace(contactId, impl);
        accepted.push_back(impl);
    }
    return accepted;
}

model::PersonContactArray ContactModel::updatePersonContactsInMemory(const model::PersonContactArray& persons)
{
    model::PersonContactArray accepted;
    accepted.reserve(persons.size());
    std::scoped_lock lock(mMutex);
    for (const auto& p : persons)
    {
        if (!p)
        {
            continue;
        }
        auto it = mPersonContacts.find(p->getContactId());
        if (it == mPersonContacts.end())
        {
            continue;
        }
        it->second->setPersonName(p->getPersonName());
        it->second->setContactStatus(p->getContactStatus());
        accepted.push_back(it->second);
    }
    return accepted;
}

std::vector<std::string> ContactModel::removePersonContactsInMemory(const std::vector<std::string>& contactIds)
{
    std::vector<std::string> accepted;
    accepted.reserve(contactIds.size());
    std::scoped_lock lock(mMutex);
    for (const auto& id : contactIds)
    {
        if (mPersonContacts.erase(id) > 0)
        {
            accepted.push_back(id);
        }
    }
    return accepted;
}

model::GroupContactArray ContactModel::addGroupContactsInMemory(const model::GroupContactArray& groups)
{
    model::GroupContactArray accepted;
    accepted.reserve(groups.size());
    std::scoped_lock lock(mMutex);
    for (const auto& g : groups)
    {
        if (!g)
        {
            continue;
        }
        const std::string contactId = g->getContactId();
        if (contactId.empty() || mGroupContacts.find(contactId) != mGroupContacts.end())
        {
            continue;
        }
        auto impl = std::make_shared<model::GroupContact>(contactId);
        impl->setGroupName(g->getGroupName());
        impl->setContactStatus(g->getContactStatus());
        mGroupContacts.emplace(contactId, impl);
        accepted.push_back(impl);
    }
    return accepted;
}

model::GroupContactArray ContactModel::updateGroupContactsInMemory(const model::GroupContactArray& groups)
{
    model::GroupContactArray accepted;
    accepted.reserve(groups.size());
    std::scoped_lock lock(mMutex);
    for (const auto& g : groups)
    {
        if (!g)
        {
            continue;
        }
        auto it = mGroupContacts.find(g->getContactId());
        if (it == mGroupContacts.end())
        {
            continue;
        }
        it->second->setGroupName(g->getGroupName());
        it->second->setContactStatus(g->getContactStatus());
        accepted.push_back(it->second);
    }
    return accepted;
}

std::vector<std::string> ContactModel::removeGroupContactsInMemory(const std::vector<std::string>& contactIds)
{
    std::vector<std::string> accepted;
    accepted.reserve(contactIds.size());
    std::scoped_lock lock(mMutex);
    for (const auto& id : contactIds)
    {
        if (mGroupContacts.erase(id) > 0)
        {
            accepted.push_back(id);
        }
    }
    return accepted;
}

model::ContactRelationArray ContactModel::addContactRelationsInMemory(const model::ContactRelationArray& relations)
{
    model::ContactRelationArray accepted;
    accepted.reserve(relations.size());
    std::scoped_lock lock(mMutex);
    for (const auto& r : relations)
    {
        if (!r)
        {
            continue;
        }
        const std::string childId = r->getChildId();
        if (childId.empty() || mContactRelations.find(childId) != mContactRelations.end())
        {
            continue;
        }
        auto impl = std::make_shared<model::ContactRelation>(childId, r->getParentId());
        impl->setRelationType(r->getRelationType());
        mContactRelations.emplace(childId, impl);
        accepted.push_back(impl);
    }
    return accepted;
}

model::ContactRelationArray ContactModel::updateContactRelationsInMemory(const model::ContactRelationArray& relations)
{
    model::ContactRelationArray accepted;
    accepted.reserve(relations.size());
    std::scoped_lock lock(mMutex);
    for (const auto& r : relations)
    {
        if (!r)
        {
            continue;
        }
        auto it = mContactRelations.find(r->getChildId());
        if (it == mContactRelations.end())
        {
            continue;
        }
        it->second->setParentId(r->getParentId());
        it->second->setRelationType(r->getRelationType());
        accepted.push_back(it->second);
    }
    return accepted;
}

std::vector<std::string> ContactModel::removeContactRelationsInMemory(const std::vector<std::string>& childIds)
{
    std::vector<std::string> accepted;
    accepted.reserve(childIds.size());
    std::scoped_lock lock(mMutex);
    for (const auto& id : childIds)
    {
        if (mContactRelations.erase(id) > 0)
        {
            accepted.push_back(id);
        }
    }
    return accepted;
}

// ===== Public write: memory first, then persist =====

model::PersonContactArray ContactModel::addPersonContacts(const model::PersonContactArray& persons)
{
    auto accepted = addPersonContactsInMemory(persons);
    if (!accepted.empty())
    {
        mContactDBAccess->insertPersonContacts(accepted);
    }
    return accepted;
}

model::PersonContactArray ContactModel::updatePersonContacts(const model::PersonContactArray& persons)
{
    auto accepted = updatePersonContactsInMemory(persons);
    for (const auto& p : accepted)
    {
        mContactDBAccess->updatePersonContact(p);
    }
    return accepted;
}

std::vector<std::string> ContactModel::removePersonContacts(const std::vector<std::string>& contactIds)
{
    auto accepted = removePersonContactsInMemory(contactIds);
    for (const auto& id : accepted)
    {
        mContactDBAccess->deletePersonContact(id);
    }
    return accepted;
}

model::GroupContactArray ContactModel::addGroupContacts(const model::GroupContactArray& groups)
{
    auto accepted = addGroupContactsInMemory(groups);
    if (!accepted.empty())
    {
        mContactDBAccess->insertGroupContacts(accepted);
    }
    return accepted;
}

model::GroupContactArray ContactModel::updateGroupContacts(const model::GroupContactArray& groups)
{
    auto accepted = updateGroupContactsInMemory(groups);
    for (const auto& g : accepted)
    {
        mContactDBAccess->updateGroupContact(g);
    }
    return accepted;
}

std::vector<std::string> ContactModel::removeGroupContacts(const std::vector<std::string>& contactIds)
{
    auto accepted = removeGroupContactsInMemory(contactIds);
    for (const auto& id : accepted)
    {
        mContactDBAccess->deleteGroupContact(id);
    }
    return accepted;
}

model::ContactRelationArray ContactModel::addContactRelations(const model::ContactRelationArray& relations)
{
    auto accepted = addContactRelationsInMemory(relations);
    if (!accepted.empty())
    {
        mContactDBAccess->insertContactRelations(accepted);
    }
    return accepted;
}

model::ContactRelationArray ContactModel::updateContactRelations(const model::ContactRelationArray& relations)
{
    auto accepted = updateContactRelationsInMemory(relations);
    for (const auto& r : accepted)
    {
        mContactDBAccess->updateContactRelation(r);
    }
    return accepted;
}

std::vector<std::string> ContactModel::removeContactRelations(const std::vector<std::string>& childIds)
{
    auto accepted = removeContactRelationsInMemory(childIds);
    for (const auto& id : accepted)
    {
        mContactDBAccess->deleteContactRelation(id);
    }
    return accepted;
}

// ===== Lifecycle =====

void ContactModel::onDatabaseReady(const std::string& databaseId)
{
    SERVICE_LOG_DEBUG("Database ready, databaseId:" << databaseId);
    mContactDBAccess->setDatabaseId(databaseId);

    mContactDBAccess->loadPersonContacts(
        [this](const model::PersonContactArray& persons)
        {
            addPersonContactsInMemory(persons);
        });
    mContactDBAccess->loadGroupContacts(
        [this](const model::GroupContactArray& groups)
        {
            addGroupContactsInMemory(groups);
        });
    mContactDBAccess->loadContactRelations(
        [this](const model::ContactRelationArray& relations)
        {
            addContactRelationsInMemory(relations);
        });
}

} // namespace ucf::service

