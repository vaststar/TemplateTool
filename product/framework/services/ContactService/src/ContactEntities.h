#pragma once

#include <string>
#include <vector>
#include <mutex>

#include <ucf/Services/ContactService/IContactEntities.h>

namespace ucf::service::model{
class PersonContact: public IPersonContact
{
public:
    explicit PersonContact(const std::string& id);
    ~PersonContact();

    virtual std::string getContactId() const override;

    virtual std::string getPersonName() const override;
    void setPersonName(const std::string& name);

    virtual IContact::ContactStatus getContactStatus() const override;
    void setContactStatus(IContact::ContactStatus status);
private:
    mutable std::mutex mDataMutex;
    const std::string mContactId;
    std::string mPersonName;
    IContact::ContactStatus mStatus{IContact::ContactStatus::Active};
};
using PersonContactPtr = std::shared_ptr<PersonContact>;

class GroupContact: public IGroupContact
{
public:
    explicit GroupContact(const std::string& id);
    virtual std::string getContactId() const override;

    virtual std::string getGroupName() const override;
    void setGroupName(const std::string& groupName);

    virtual IGroupContact::GroupType getGroupType() const override;
    void setGroupType(IGroupContact::GroupType type);

    virtual IContact::ContactStatus getContactStatus() const override;
    void setContactStatus(IContact::ContactStatus status);

protected:
    mutable std::mutex mDataMutex;
    const std::string mContactId;
    IContact::ContactStatus mStatus{IContact::ContactStatus::Active};
    std::string mGroupName;
    IGroupContact::GroupType mGroupType{IGroupContact::GroupType::Department};
};
using GroupContactPtr = std::shared_ptr<GroupContact>;

// Department-typed concrete group; adds org-chart fields stored in its own sub-table.
class DepartmentGroupContact: public GroupContact, public IDepartmentGroup
{
public:
    explicit DepartmentGroupContact(const std::string& id);

    // Disambiguate the diamond by routing IGroupContact / IContact methods through
    // the GroupContact base; getGroupType() is hardcoded to Department so this
    // class always reports its own type regardless of any setGroupType() call on
    // the base.
    std::string                 getContactId()      const override { return GroupContact::getContactId(); }
    std::string                 getGroupName()      const override { return GroupContact::getGroupName(); }
    IGroupContact::GroupType    getGroupType()      const override { return IGroupContact::GroupType::Department; }
    IContact::ContactStatus     getContactStatus()  const override { return GroupContact::getContactStatus(); }

    std::string getManagerId() const override;
    int         getHeadcount() const override;
    void setManagerId(const std::string& managerId);
    void setHeadcount(int headcount);

private:
    std::string mManagerId;
    int         mHeadcount{0};
};
using DepartmentGroupContactPtr = std::shared_ptr<DepartmentGroupContact>;

// Team-typed concrete group; adds team-charter fields stored in its own sub-table.
class TeamGroupContact: public GroupContact, public ITeamGroup
{
public:
    explicit TeamGroupContact(const std::string& id);

    std::string                 getContactId()      const override { return GroupContact::getContactId(); }
    std::string                 getGroupName()      const override { return GroupContact::getGroupName(); }
    IGroupContact::GroupType    getGroupType()      const override { return IGroupContact::GroupType::Team; }
    IContact::ContactStatus     getContactStatus()  const override { return GroupContact::getContactStatus(); }

    std::string getTeamLeadId() const override;
    std::string getMission()    const override;
    void setTeamLeadId(const std::string& teamLeadId);
    void setMission(const std::string& mission);

private:
    std::string mTeamLeadId;
    std::string mMission;
};
using TeamGroupContactPtr = std::shared_ptr<TeamGroupContact>;

class ContactRelation: public IContactRelation
{
public:
    // relationId 是这一行的稳定主键，必须非空（由 service 在持久化前生成或沿用 caller 提供的）。
    // childId 必须非空；parentId 为空字符串表示没有父节点（root）。
    ContactRelation(const std::string& relationId,
                    const std::string& childId,
                    const std::string& parentId);
    ~ContactRelation() override;

    virtual std::string getRelationId() const override;

    virtual std::string getChildId() const override;

    virtual std::string getParentId() const override;
    void setParentId(const std::string& parentId);

    virtual IContactRelation::RelationType getRelationType() const override;
    void setRelationType(IContactRelation::RelationType type);
private:
    mutable std::mutex mDataMutex;
    const std::string mRelationId;
    const std::string mChildId;
    std::string mParentId;
    IContactRelation::RelationType mRelationType{IContactRelation::RelationType::Department};
};

using ContactRelationPtr = std::shared_ptr<ContactRelation>;
}
