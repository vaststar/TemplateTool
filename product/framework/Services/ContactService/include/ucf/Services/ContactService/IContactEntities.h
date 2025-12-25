#pragma once

#include <string>
#include <vector>

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

namespace ucf::service::model{

class SERVICE_EXPORT IContact
{
public:
    enum class ContactStatus {
        Active = 0,      // 活跃
        Inactive = 1,    // 非活跃（暂时停用）
        Deleted = 2,     // 已删除（软删除）
        Archived = 3     // 已归档
    };
    virtual ~IContact() = default;

    // Returns the unique identifier of the contact, only used for identification purposes.
    virtual std::string getContactId() const = 0;
    virtual ContactStatus getContactStatus() const = 0;
};
using IContactPtr = std::shared_ptr<IContact>;

// Represents a contact for a person, which can be used to store personal information.
class SERVICE_EXPORT IPersonContact: public IContact
{
public:
    virtual std::string getPersonName() const = 0;
};
using IPersonContactPtr = std::shared_ptr<IPersonContact>;

// Represents a contact for a group, which can be used to store group information.
class SERVICE_EXPORT IGroupContact: public IContact
{
public:
    virtual std::string getGroupName() const = 0;
};
using IGroupContactPtr = std::shared_ptr<IGroupContact>;

class SERVICE_EXPORT IContactRelation
{
public:
    enum class RelationType {
        Department,     // 部门关系：员工属于部门
        Reporting,      // 汇报关系：下级向上级汇报
        Project,        // 项目关系：项目组成员
        Mentor,         // 导师关系：导师-学员
        Collaboration   // 协作关系：跨部门协作
    };
    virtual ~IContactRelation() = default;

    virtual std::string getChildId() const = 0;

    virtual std::string getParentId() const = 0;

    virtual RelationType getRelationType() const = 0;
};

using IContactRelationPtr  = std::shared_ptr<IContactRelation>;
using ContactRelationArray = std::vector<IContactRelationPtr>;
}
