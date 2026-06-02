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
    SERVICE_LOG_DEBUG("addPersonContacts requested, count:" << persons.size());
    auto accepted = addPersonContactsInMemory(persons);
    SERVICE_LOG_DEBUG("addPersonContacts accepted, count:" << accepted.size());
    if (!accepted.empty())
    {
        mContactDBAccess->insertPersonContacts(accepted);
        if (auto sink = mNotificationSink.lock())
        {
            sink->onPersonContactsAdded(accepted, ContactNotificationSource::Local);
        }
    }
    return accepted;
}

model::PersonContactArray ContactModel::updatePersonContacts(const model::PersonContactArray& persons)
{
    SERVICE_LOG_DEBUG("updatePersonContacts requested, count:" << persons.size());
    auto accepted = updatePersonContactsInMemory(persons);
    SERVICE_LOG_DEBUG("updatePersonContacts accepted, count:" << accepted.size());
    for (const auto& p : accepted)
    {
        mContactDBAccess->updatePersonContact(p);
    }
    if (!accepted.empty())
    {
        if (auto sink = mNotificationSink.lock())
        {
            sink->onPersonContactsUpdated(accepted, ContactNotificationSource::Local);
        }
    }
    return accepted;
}

std::vector<std::string> ContactModel::removePersonContacts(const std::vector<std::string>& contactIds)
{
    SERVICE_LOG_DEBUG("removePersonContacts requested, count:" << contactIds.size());
    auto accepted = removePersonContactsInMemory(contactIds);
    SERVICE_LOG_DEBUG("removePersonContacts accepted, count:" << accepted.size());
    for (const auto& id : accepted)
    {
        mContactDBAccess->deletePersonContact(id);
    }
    if (!accepted.empty())
    {
        if (auto sink = mNotificationSink.lock())
        {
            sink->onPersonContactsRemoved(accepted, ContactNotificationSource::Local);
        }
    }
    return accepted;
}

model::GroupContactArray ContactModel::addGroupContacts(const model::GroupContactArray& groups)
{
    SERVICE_LOG_DEBUG("addGroupContacts requested, count:" << groups.size());
    auto accepted = addGroupContactsInMemory(groups);
    SERVICE_LOG_DEBUG("addGroupContacts accepted, count:" << accepted.size());
    if (!accepted.empty())
    {
        mContactDBAccess->insertGroupContacts(accepted);
        if (auto sink = mNotificationSink.lock())
        {
            sink->onGroupContactsAdded(accepted, ContactNotificationSource::Local);
        }
    }
    return accepted;
}

model::GroupContactArray ContactModel::updateGroupContacts(const model::GroupContactArray& groups)
{
    SERVICE_LOG_DEBUG("updateGroupContacts requested, count:" << groups.size());
    auto accepted = updateGroupContactsInMemory(groups);
    SERVICE_LOG_DEBUG("updateGroupContacts accepted, count:" << accepted.size());
    for (const auto& g : accepted)
    {
        mContactDBAccess->updateGroupContact(g);
    }
    if (!accepted.empty())
    {
        if (auto sink = mNotificationSink.lock())
        {
            sink->onGroupContactsUpdated(accepted, ContactNotificationSource::Local);
        }
    }
    return accepted;
}

std::vector<std::string> ContactModel::removeGroupContacts(const std::vector<std::string>& contactIds)
{
    SERVICE_LOG_DEBUG("removeGroupContacts requested, count:" << contactIds.size());
    auto accepted = removeGroupContactsInMemory(contactIds);
    SERVICE_LOG_DEBUG("removeGroupContacts accepted, count:" << accepted.size());
    for (const auto& id : accepted)
    {
        mContactDBAccess->deleteGroupContact(id);
    }
    if (!accepted.empty())
    {
        if (auto sink = mNotificationSink.lock())
        {
            sink->onGroupContactsRemoved(accepted, ContactNotificationSource::Local);
        }
    }
    return accepted;
}

model::ContactRelationArray ContactModel::addContactRelations(const model::ContactRelationArray& relations)
{
    SERVICE_LOG_DEBUG("addContactRelations requested, count:" << relations.size());
    auto accepted = addContactRelationsInMemory(relations);
    SERVICE_LOG_DEBUG("addContactRelations accepted, count:" << accepted.size());
    if (!accepted.empty())
    {
        mContactDBAccess->insertContactRelations(accepted);
        if (auto sink = mNotificationSink.lock())
        {
            sink->onContactRelationsAdded(accepted, ContactNotificationSource::Local);
        }
    }
    return accepted;
}

model::ContactRelationArray ContactModel::updateContactRelations(const model::ContactRelationArray& relations)
{
    SERVICE_LOG_DEBUG("updateContactRelations requested, count:" << relations.size());
    auto accepted = updateContactRelationsInMemory(relations);
    SERVICE_LOG_DEBUG("updateContactRelations accepted, count:" << accepted.size());
    for (const auto& r : accepted)
    {
        mContactDBAccess->updateContactRelation(r);
    }
    if (!accepted.empty())
    {
        if (auto sink = mNotificationSink.lock())
        {
            sink->onContactRelationsUpdated(accepted, ContactNotificationSource::Local);
        }
    }
    return accepted;
}

std::vector<std::string> ContactModel::removeContactRelations(const std::vector<std::string>& childIds)
{
    SERVICE_LOG_DEBUG("removeContactRelations requested, count:" << childIds.size());
    auto accepted = removeContactRelationsInMemory(childIds);
    SERVICE_LOG_DEBUG("removeContactRelations accepted, count:" << accepted.size());
    for (const auto& id : accepted)
    {
        mContactDBAccess->deleteContactRelation(id);
    }
    if (!accepted.empty())
    {
        if (auto sink = mNotificationSink.lock())
        {
            sink->onContactRelationsRemoved(accepted, ContactNotificationSource::Local);
        }
    }
    return accepted;
}

// ===== Lifecycle =====

void ContactModel::setNotificationSink(std::weak_ptr<IContactNotificationSink> sink)
{
    SERVICE_LOG_DEBUG("setNotificationSink");
    mNotificationSink = std::move(sink);
}

void ContactModel::bindDatabase(const std::string& databaseId)
{
    if (databaseId.empty())
    {
        SERVICE_LOG_ERROR("bindDatabase ignored: empty databaseId");
        return;
    }

    if (mLoadStage.load() != LoadStage::Uninit)
    {
        const auto& current = mContactDBAccess->getDatabaseId();
        if (current != databaseId)
        {
            SERVICE_LOG_ERROR("bindDatabase rejected: rebinding to a different db is not supported"
                              << ", current:" << current
                              << ", incoming:" << databaseId);
        }
        else
        {
            SERVICE_LOG_DEBUG("bindDatabase noop, same databaseId:" << databaseId);
        }
        return;
    }

    mContactDBAccess->setDatabaseId(databaseId);
    mLoadStage.store(LoadStage::DbBound);
    SERVICE_LOG_DEBUG("bindDatabase done, databaseId:" << databaseId);
}

void ContactModel::loadContactDirectory()
{
    auto stage = mLoadStage.load();
    if (stage == LoadStage::Uninit)
    {
        SERVICE_LOG_ERROR("loadContactDirectory ignored: database not bound yet");
        finishLoadFailure(ContactDirectoryLoadError::DatabaseNotBound);
        return;
    }
    if (stage == LoadStage::Loading || stage == LoadStage::Ready)
    {
        SERVICE_LOG_DEBUG("loadContactDirectory ignored, stage:" << static_cast<int>(stage));
        return;
    }
    if (!mLoadStage.compare_exchange_strong(stage, LoadStage::Loading))
    {
        SERVICE_LOG_DEBUG("loadContactDirectory raced, stage advanced by another thread");
        return;
    }
    SERVICE_LOG_DEBUG("loadContactDirectory started");

    struct LoadContext
    {
        std::atomic<int> remaining{3};
    };
    auto ctx = std::make_shared<LoadContext>();

    auto onChunkDone = [this, ctx](const char* chunkName, std::size_t count)
    {
        SERVICE_LOG_DEBUG("loadContactDirectory chunk done, chunk:" << chunkName
                          << ", count:" << count
                          << ", remaining:" << (ctx->remaining.load() - 1));
        if (ctx->remaining.fetch_sub(1) == 1)
        {
            finishLoadSuccess();
        }
    };

    mContactDBAccess->loadPersonContacts(
        [this, onChunkDone](const model::PersonContactArray& persons)
        {
            addPersonContactsInMemory(persons);
            onChunkDone("persons", persons.size());
        });
    mContactDBAccess->loadGroupContacts(
        [this, onChunkDone](const model::GroupContactArray& groups)
        {
            addGroupContactsInMemory(groups);
            onChunkDone("groups", groups.size());
        });
    mContactDBAccess->loadContactRelations(
        [this, onChunkDone](const model::ContactRelationArray& relations)
        {
            addContactRelationsInMemory(relations);
            onChunkDone("relations", relations.size());
        });
}

bool ContactModel::isContactDirectoryReady() const
{
    return mLoadStage.load() == LoadStage::Ready;
}

void ContactModel::finishLoadSuccess()
{
    mLoadStage.store(LoadStage::Ready);
    SERVICE_LOG_DEBUG("loadContactDirectory finished, success:true");
    if (auto sink = mNotificationSink.lock())
    {
        sink->onDirectoryLoaded();
    }
}

void ContactModel::finishLoadFailure(ContactDirectoryLoadError error)
{
    mLoadStage.store(LoadStage::Failed);
    SERVICE_LOG_ERROR("loadContactDirectory finished, success:false, error:" << static_cast<int>(error));
    if (auto sink = mNotificationSink.lock())
    {
        sink->onDirectoryLoadFailed(error);
    }
}

} // namespace ucf::service

