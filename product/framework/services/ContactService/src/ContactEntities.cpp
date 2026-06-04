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

IGroupContact::GroupType GroupContact::getGroupType() const
{
    std::scoped_lock lock(mDataMutex);
    return mGroupType;
}

void GroupContact::setGroupType(IGroupContact::GroupType type)
{
    std::scoped_lock lock(mDataMutex);
    mGroupType = type;
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

// ===== DepartmentGroupContact =====

DepartmentGroupContact::DepartmentGroupContact(const std::string& id)
    : GroupContact(id)
{
}

std::string DepartmentGroupContact::getManagerId() const
{
    std::scoped_lock lock(mDataMutex);
    return mManagerId;
}

int DepartmentGroupContact::getHeadcount() const
{
    std::scoped_lock lock(mDataMutex);
    return mHeadcount;
}

void DepartmentGroupContact::setManagerId(const std::string& managerId)
{
    std::scoped_lock lock(mDataMutex);
    mManagerId = managerId;
}

void DepartmentGroupContact::setHeadcount(int headcount)
{
    std::scoped_lock lock(mDataMutex);
    mHeadcount = headcount;
}

// ===== TeamGroupContact =====

TeamGroupContact::TeamGroupContact(const std::string& id)
    : GroupContact(id)
{
}

std::string TeamGroupContact::getTeamLeadId() const
{
    std::scoped_lock lock(mDataMutex);
    return mTeamLeadId;
}

std::string TeamGroupContact::getMission() const
{
    std::scoped_lock lock(mDataMutex);
    return mMission;
}

void TeamGroupContact::setTeamLeadId(const std::string& teamLeadId)
{
    std::scoped_lock lock(mDataMutex);
    mTeamLeadId = teamLeadId;
}

void TeamGroupContact::setMission(const std::string& mission)
{
    std::scoped_lock lock(mDataMutex);
    mMission = mission;
}

ContactRelation::ContactRelation(const std::string& relationId,
                                 const std::string& childId,
                                 const std::string& parentId)
    : mRelationId(relationId)
    , mChildId(childId)
    , mParentId(parentId)
{
}

ContactRelation::~ContactRelation()
{
}

std::string ContactRelation::getRelationId() const
{
    return mRelationId;
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
