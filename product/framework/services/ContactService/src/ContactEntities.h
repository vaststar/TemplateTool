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
private:
    mutable std::mutex mDataMutex;
    const std::string mContactId;
    IContact::ContactStatus mStatus{IContact::ContactStatus::Active};
    std::string mGroupName;
    IGroupContact::GroupType mGroupType{IGroupContact::GroupType::Department};
};
using GroupContactPtr = std::shared_ptr<GroupContact>;

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
