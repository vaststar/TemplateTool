#include <algorithm>
#include <set>
#include "ContactEntities.h"

namespace ucf::service::model{

PersonContact::PersonContact(const std::string& id)
    : mContactId(id)
{

}
PersonContact::~PersonContact()
{

}
std::string PersonContact::getContactId() const
{
    return mContactId;
}

std::string PersonContact::getPersonName() const
{
    std::scoped_lock lock(mDataMutex);
    return mPersonName;
}

void PersonContact::setPersonName(const std::string& name)
{
    std::scoped_lock lock(mDataMutex);
    mPersonName = name;
}

IContact::ContactStatus PersonContact::getContactStatus() const
{
    std::scoped_lock lock(mDataMutex);
    return mStatus;
}

void PersonContact::setContactStatus(ContactStatus status)
{
    std::scoped_lock lock(mDataMutex);
    mStatus = status;
}

GroupContact::GroupContact(const std::string& id)
    : mContactId(id)
{
}

std::string GroupContact::getContactId() const
{
    return mContactId;
}

std::string GroupContact::getGroupName() const
{
    std::scoped_lock lock(mDataMutex);
    return mGroupName;
}

void GroupContact::setGroupName(const std::string& groupName)
{
    std::scoped_lock lock(mDataMutex);
    mGroupName = groupName;
}

IContact::ContactStatus GroupContact::getContactStatus() const
{
    std::scoped_lock lock(mDataMutex);
    return mStatus;
}

void GroupContact::setContactStatus(ContactStatus status)
{
    std::scoped_lock lock(mDataMutex);
    mStatus = status;
}

ContactRelation::ContactRelation(const std::string& childId,
                                 const std::string& parentId)
    : mChildId(childId)
    , mParentId(parentId)
{
}

ContactRelation::~ContactRelation()
{
}

std::string ContactRelation::getChildId() const
{
    return mChildId;
}

std::string ContactRelation::getParentId() const
{
    std::scoped_lock lock(mDataMutex);
    return mParentId;
}

void ContactRelation::setParentId(const std::string& parentId)
{
    std::scoped_lock lock(mDataMutex);
    mParentId = parentId;
}

IContactRelation::RelationType ContactRelation::getRelationType() const
{
    std::scoped_lock lock(mDataMutex);
    return mRelationType;
}

void ContactRelation::setRelationType(IContactRelation::RelationType type)
{
    std::scoped_lock lock(mDataMutex);
    mRelationType = type;
}
} // namespace ucf::service::model
