#include "ContactModel.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <set>


#include <ucf/CoreFramework/ICoreFramework.h>

#include "ContactServiceLogger.h"
// #include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/IDatabaseWrapper.h>

namespace ucf::service{


ContactModel::ContactModel(ucf::framework::ICoreFrameworkWPtr coreFramework)
:  mCoreFrameworkWPtr(coreFramework)
{
    SERVICE_LOG_DEBUG("create ContactModel, address:"  << this);
}

std::vector<model::IPersonContactPtr> ContactModel::getPersonContacts() const
{
    std::vector<model::IPersonContactPtr> results;
    std::scoped_lock loc(mContactMutex);
    std::transform(mPersonContacts.begin(), mPersonContacts.end(), std::back_inserter(results), [](const auto& contactPair) -> model::IPersonContactPtr {
        return contactPair.second;
    });
    return results;
}

void ContactModel::deletePersonContacts(const std::initializer_list<std::string>& contactIds)
{
    std::scoped_lock loc(mContactMutex);
    for (const auto& contactId : contactIds)
    {
        if (auto iter = mPersonContacts.find(contactId); iter != mPersonContacts.end())
        {
            iter->second->setContactStatus(model::IContact::ContactStatus::Deleted);
        }
    }
}

void ContactModel::addPersonContact(const model::PersonContactPtr& contact)
{
    std::scoped_lock loc(mContactMutex);
    if (auto iter = mPersonContacts.find(contact->getContactId()); iter != mPersonContacts.end())
    {
        SERVICE_LOG_DEBUG("Contact already exists, contactId:" << contact->getContactId());
        return;
    }
    mPersonContacts.emplace(contact->getContactId(), contact);
}

model::IPersonContactPtr ContactModel::getPersonContact(const std::string& contactId) const
{
    std::scoped_lock loc(mContactMutex);
    if (auto iter = mPersonContacts.find(contactId); iter != mPersonContacts.end())
    {
        return iter->second;
    }
    return nullptr;
}

std::vector<model::IGroupContactPtr> ContactModel::getGroupContacts() const
{
    std::vector<model::IGroupContactPtr> results;
    std::scoped_lock loc(mContactMutex);
    std::transform(mGroupContacts.begin(), mGroupContacts.end(), std::back_inserter(results), [](const auto& contactPair) -> model::IGroupContactPtr {
        return contactPair.second;
    });
    return results;
}

model::IGroupContactPtr ContactModel::getGroupContact(const std::string& contactId) const
{
    std::scoped_lock loc(mContactMutex);
    if (auto iter = mGroupContacts.find(contactId); iter != mGroupContacts.end())
    {
        return iter->second;
    }
    return nullptr;
}

void ContactModel::addGroupContact(const model::GroupContactPtr& contact)
{
    std::scoped_lock loc(mContactMutex);
    if (auto iter = mGroupContacts.find(contact->getContactId()); iter != mGroupContacts.end())
    {
        SERVICE_LOG_DEBUG("Group Contact already exists, contactId:" << contact->getContactId());
        return;
    }
    mGroupContacts.emplace(contact->getContactId(), contact);
}

void ContactModel::deleteGroupContacts(const std::initializer_list<std::string>& contactIds)
{
    std::scoped_lock loc(mContactMutex);
    for (const auto& contactId : contactIds)
    {
        if (auto iter = mGroupContacts.find(contactId); iter != mGroupContacts.end())
        {
            iter->second->setContactStatus(model::IContact::ContactStatus::Deleted);
        }
    }
}

std::vector<model::IContactRelationPtr> ContactModel::getContactRelations() const
{
    std::vector<model::IContactRelationPtr> results;
    std::scoped_lock loc(mContactMutex);
    std::transform(mContactRelations.begin(), mContactRelations.end(), std::back_inserter(results), [](const auto& relation) -> model::IContactRelationPtr {
        return relation;
    });
    return results;
}

void ContactModel::addContactRelation(const model::ContactRelationPtr& relation)
{
    std::scoped_lock loc(mContactMutex);
    auto it = std::find_if(mContactRelations.begin(), mContactRelations.end(),
                           [&relation](const model::ContactRelationPtr& existingRelation) {
                               return existingRelation->getChildId() == relation->getChildId();
                           });
    if (it != mContactRelations.end())
    {
        SERVICE_LOG_DEBUG("Contact relation already exists for childId:" << relation->getChildId());
        return;
    }
    mContactRelations.push_back(relation);
}

void ContactModel::deleteContactRelations(const std::initializer_list<std::string>& childIds)
{
    std::scoped_lock loc(mContactMutex);
    for (const auto& childId : childIds)
    {
        auto it = std::remove_if(mContactRelations.begin(), mContactRelations.end(),
                                 [&childId](const model::ContactRelationPtr& relation) {
                                     return relation->getChildId() == childId;
                                 });
        if (it != mContactRelations.end())
        {
            mContactRelations.erase(it, mContactRelations.end());
        }
    }
}

void ContactModel::modifyContactRelation(const std::string& childId, const std::string& newParentId)
{
    std::scoped_lock loc(mContactMutex);
    auto it = std::find_if(mContactRelations.begin(), mContactRelations.end(),
                           [&childId](const model::ContactRelationPtr& relation) {
                               return relation->getChildId() == childId;
                           });
    if (it != mContactRelations.end())
    {
        (*it)->setParentId(newParentId);
    }
}
} // namespace ucf::service