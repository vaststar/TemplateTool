#include "ContactModel.h"

#include <ucf/Utilities/UUIDUtils/UUIDUtils.h>

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

model::GroupContactArray ContactModel::getGroupContacts(
    std::optional<model::IGroupContact::GroupType> groupType) const
{
    model::GroupContactArray result;
    std::scoped_lock lock(mMutex);
    result.reserve(mGroupContacts.size());
    for (const auto& [_, g] : mGroupContacts)
    {
        if (groupType.has_value() && g->getGroupType() != *groupType)
        {
            continue;
        }
        result.push_back(g);
    }
    return result;
}

model::ContactRelationArray ContactModel::getContactRelations(
    std::optional<model::IContactRelation::RelationType> relationType) const
{
    model::ContactRelationArray result;
    std::scoped_lock lock(mMutex);
    result.reserve(mContactRelations.size());
    for (const auto& [_, r] : mContactRelations)
    {
        if (relationType.has_value() && r->getRelationType() != *relationType)
        {
            continue;
        }
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
        // CTI: build the concrete subclass matching g->getGroupType(), then copy
        // typed sub-fields when the caller passed in a typed interface.
        model::GroupContactPtr impl;
        switch (g->getGroupType())
        {
        case model::IGroupContact::GroupType::Department:
            impl = std::make_shared<model::DepartmentGroupContact>(contactId);
            break;
        case model::IGroupContact::GroupType::Team:
            impl = std::make_shared<model::TeamGroupContact>(contactId);
            break;
        case model::IGroupContact::GroupType::Folder:
            impl = std::make_shared<model::FolderGroupContact>(contactId);
            break;
        default:
            impl = std::make_shared<model::GroupContact>(contactId);
            impl->setGroupType(g->getGroupType());
            break;
        }
        impl->setGroupName(g->getGroupName());
        impl->setContactStatus(g->getContactStatus());
        if (auto srcDept = std::dynamic_pointer_cast<model::IDepartmentGroup>(g))
        {
            if (auto dstDept = std::dynamic_pointer_cast<model::DepartmentGroupContact>(impl))
            {
                dstDept->setManagerId(srcDept->getManagerId());
                dstDept->setHeadcount(srcDept->getHeadcount());
            }
        }
        else if (auto srcTeam = std::dynamic_pointer_cast<model::ITeamGroup>(g))
        {
            if (auto dstTeam = std::dynamic_pointer_cast<model::TeamGroupContact>(impl))
            {
                dstTeam->setTeamLeadId(srcTeam->getTeamLeadId());
                dstTeam->setMission(srcTeam->getMission());
            }
        }
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
        // We don't support changing GroupType on update (would require swapping the
        // concrete subclass + migrating between sub-tables); skip those silently.
        if (it->second->getGroupType() != g->getGroupType())
        {
            SERVICE_LOG_WARN("updateGroupContactsInMemory: GroupType change not supported, contactId=" << g->getContactId());
            continue;
        }
        it->second->setGroupName(g->getGroupName());
        it->second->setContactStatus(g->getContactStatus());
        if (auto srcDept = std::dynamic_pointer_cast<model::IDepartmentGroup>(g))
        {
            if (auto dstDept = std::dynamic_pointer_cast<model::DepartmentGroupContact>(it->second))
            {
                dstDept->setManagerId(srcDept->getManagerId());
                dstDept->setHeadcount(srcDept->getHeadcount());
            }
        }
        else if (auto srcTeam = std::dynamic_pointer_cast<model::ITeamGroup>(g))
        {
            if (auto dstTeam = std::dynamic_pointer_cast<model::TeamGroupContact>(it->second))
            {
                dstTeam->setTeamLeadId(srcTeam->getTeamLeadId());
                dstTeam->setMission(srcTeam->getMission());
            }
        }
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
        const std::string childId  = r->getChildId();
        const std::string parentId = r->getParentId();
        const auto        type     = r->getRelationType();
        if (childId.empty())
        {
            continue;
        }
        // Caller may leave the id empty for a brand-new row; we mint one here so that
        // every persisted relation has a stable surrogate key.
        std::string relationId = r->getRelationId();
        if (relationId.empty())
        {
            relationId = ucf::utilities::UUIDUtils::generateUUID();
        }
        if (mContactRelations.find(relationId) != mContactRelations.end())
        {
            continue;
        }
        auto impl = std::make_shared<model::ContactRelation>(relationId, childId, parentId);
        impl->setRelationType(type);
        mContactRelations.emplace(relationId, impl);
        accepted.push_back(impl);
    }
    return accepted;
}

model::ContactRelationArray ContactModel::updateContactRelationsInMemory(const model::ContactRelationArray& relations)
{
    // Identify each row by its relationId; mutable fields are parentId and relationType
    // (a "move" ⇒ change parentId; a "reclassify" ⇒ change type). Rows whose id is empty
    // or unknown are silently skipped — the caller likely meant to add(), not update().
    model::ContactRelationArray accepted;
    accepted.reserve(relations.size());
    std::scoped_lock lock(mMutex);
    for (const auto& r : relations)
    {
        if (!r) continue;
        const std::string relationId = r->getRelationId();
        if (relationId.empty()) continue;
        auto it = mContactRelations.find(relationId);
        if (it == mContactRelations.end()) continue;
        it->second->setParentId(r->getParentId());
        it->second->setRelationType(r->getRelationType());
        accepted.push_back(it->second);
    }
    return accepted;
}

std::vector<std::string> ContactModel::removeContactRelationsInMemory(const std::vector<std::string>& relationIds)
{
    std::vector<std::string> accepted;
    accepted.reserve(relationIds.size());
    std::scoped_lock lock(mMutex);
    for (const auto& id : relationIds)
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
        notifySink(&IContactNotificationSink::onPersonContactsAdded, accepted, ContactNotificationSource::Local);
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
        notifySink(&IContactNotificationSink::onPersonContactsUpdated, accepted, ContactNotificationSource::Local);
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
        notifySink(&IContactNotificationSink::onPersonContactsRemoved, accepted, ContactNotificationSource::Local);
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
        notifySink(&IContactNotificationSink::onGroupContactsAdded, accepted, ContactNotificationSource::Local);
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
        notifySink(&IContactNotificationSink::onGroupContactsUpdated, accepted, ContactNotificationSource::Local);
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
        notifySink(&IContactNotificationSink::onGroupContactsRemoved, accepted, ContactNotificationSource::Local);
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
        notifySink(&IContactNotificationSink::onContactRelationsAdded, accepted, ContactNotificationSource::Local);
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
        notifySink(&IContactNotificationSink::onContactRelationsUpdated, accepted, ContactNotificationSource::Local);
    }
    return accepted;
}

std::vector<std::string> ContactModel::removeContactRelations(const std::vector<std::string>& relationIds)
{
    SERVICE_LOG_DEBUG("removeContactRelations requested, count:" << relationIds.size());
    auto accepted = removeContactRelationsInMemory(relationIds);
    SERVICE_LOG_DEBUG("removeContactRelations accepted, count:" << accepted.size());
    for (const auto& id : accepted)
    {
        mContactDBAccess->deleteContactRelation(id);
    }
    if (!accepted.empty())
    {
        notifySink(&IContactNotificationSink::onContactRelationsRemoved, accepted, ContactNotificationSource::Local);
    }
    return accepted;
}

// ===== Lifecycle =====

void ContactModel::bindDatabase(const std::string& databaseId)
{
    if (databaseId.empty())
    {
        SERVICE_LOG_ERROR("bindDatabase ignored: empty databaseId");
        return;
    }

    if (auto expected = LoadStage::Uninit; !mLoadStage.compare_exchange_strong(expected, LoadStage::DbBound))
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
    SERVICE_LOG_DEBUG("bindDatabase done, databaseId:" << databaseId);

    if (mLoadPending.exchange(false))
    {
        SERVICE_LOG_DEBUG("bindDatabase auto-promoting pending loadContactDirectory");
        loadContactDirectory();
    }
}

void ContactModel::loadContactDirectory()
{
    auto stage = mLoadStage.load();
    if (stage == LoadStage::Uninit)
    {
        // DB not bound yet: park the request. bindDatabase() will auto-promote it
        // once the DB id has been set, so callers do not need to retry.
        mLoadPending.store(true);
        SERVICE_LOG_DEBUG("loadContactDirectory deferred: database not bound yet, pending=true");
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
    // CAS so a racing finishLoadFailure cannot also fire.
    if (auto expected = LoadStage::Loading; !mLoadStage.compare_exchange_strong(expected, LoadStage::Ready))
    {
        SERVICE_LOG_DEBUG("finishLoadSuccess skipped, stage already:" << static_cast<int>(expected));
        return;
    }
    SERVICE_LOG_DEBUG("loadContactDirectory finished, success:true");
    notifySink(&IContactNotificationSink::onDirectoryLoaded);
}

void ContactModel::finishLoadFailure(ContactDirectoryLoadError error)
{
    if (auto expected = LoadStage::Loading; !mLoadStage.compare_exchange_strong(expected, LoadStage::Failed))
    {
        SERVICE_LOG_DEBUG("finishLoadFailure skipped, stage already:" << static_cast<int>(expected));
        return;
    }
    SERVICE_LOG_ERROR("loadContactDirectory finished, success:false, error:" << static_cast<int>(error));
    notifySink(&IContactNotificationSink::onDirectoryLoadFailed, error);
}

} // namespace ucf::service

