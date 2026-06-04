#pragma once

#include <string>
#include <vector>
#include <memory>

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
using IPersonContactPtr  = std::shared_ptr<IPersonContact>;
using PersonContactArray = std::vector<IPersonContactPtr>;

// Represents a contact for a group, which can be used to store group information.
// A group must declare its semantic kind via GroupType so that views (org chart, project
// list, etc.) can filter for only the groups they care about.
//
// CTI (Class Table Inheritance): IGroupContact is the common base; concrete typed
// variants (IDepartmentGroup, ITeamGroup) extend it with type-specific fields and
// live in their own sub-tables. Project / Custom currently have no typed fields and
// are represented by plain IGroupContact instances.
class SERVICE_EXPORT IGroupContact: public IContact
{
public:
    enum class GroupType {
        Department = 0, // 部门：组织架构中的一个部门
        Project    = 1, // 项目组：一个具体项目
        Team       = 2, // 临时/职能小组
        Custom     = 3, // 用户自定义分组
    };
    virtual std::string getGroupName() const = 0;
    virtual GroupType   getGroupType() const = 0;
};
using IGroupContactPtr  = std::shared_ptr<IGroupContact>;
using GroupContactArray = std::vector<IGroupContactPtr>;

// Department-typed group. Carries org-chart fields stored in its own sub-table.
class SERVICE_EXPORT IDepartmentGroup: public IGroupContact
{
public:
    // Contact id of the person managing this department; empty if unassigned.
    virtual std::string getManagerId() const = 0;
    // Authorised / target headcount for this department; 0 if unspecified.
    virtual int         getHeadcount() const = 0;
};
using IDepartmentGroupPtr = std::shared_ptr<IDepartmentGroup>;

// Team-typed group. Carries team-charter fields stored in its own sub-table.
class SERVICE_EXPORT ITeamGroup: public IGroupContact
{
public:
    // Contact id of the team lead; empty if unassigned.
    virtual std::string getTeamLeadId() const = 0;
    // Free-form short statement of what the team does.
    virtual std::string getMission()    const = 0;
};
using ITeamGroupPtr = std::shared_ptr<ITeamGroup>;

class SERVICE_EXPORT IContactRelation
{
public:
    enum class RelationType {
        Department    = 0,  // 部门关系：员工属于部门
        Reporting     = 1,  // 汇报关系：下级向上级汇报
        Project       = 2,  // 项目关系：项目组成员
        Mentor        = 3,  // 导师关系：导师-学员
        Collaboration = 4,  // 协作关系：跨部门协作
    };
    virtual ~IContactRelation() = default;

    // Stable surrogate primary key for this relation row. The triple
    // (childId, parentId, relationType) is a UNIQUE business key, but the row is
    // identified by its own id so that updates (e.g. moving a child to a new parent)
    // are real UPDATE operations rather than DELETE + INSERT. May be empty when a
    // caller submits a freshly-created relation through addContactRelations(); the
    // service then assigns a UUID before persisting.
    virtual std::string getRelationId() const = 0;

    virtual std::string getChildId() const = 0;

    virtual std::string getParentId() const = 0;

    virtual RelationType getRelationType() const = 0;
};

using IContactRelationPtr  = std::shared_ptr<IContactRelation>;
using ContactRelationArray = std::vector<IContactRelationPtr>;
}
