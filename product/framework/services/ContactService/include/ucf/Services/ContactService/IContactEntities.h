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
        Active = 0,
        Inactive = 1,
        Deleted = 2,
        Archived = 3
    };
    virtual ~IContact() = default;

    virtual std::string getContactId() const = 0;
    virtual ContactStatus getContactStatus() const = 0;
};
using IContactPtr = std::shared_ptr<IContact>;

// Person contact. Profile fields (first/last/gender/phone/email) live in the
// PersonContact CTI sub-table and may be empty for legacy rows without a sub-row.
class SERVICE_EXPORT IPersonContact: public IContact
{
public:
    enum class Gender {
        Unspecified = 0,
        Male        = 1,
        Female      = 2,
        Other       = 3,
    };

    virtual std::string getPersonName() const = 0;
    virtual std::string getFirstName()  const = 0;
    virtual std::string getLastName()   const = 0;
    virtual Gender      getGender()     const = 0;
    virtual std::string getPhone()      const = 0;
    virtual std::string getEmail()      const = 0;
};
using IPersonContactPtr  = std::shared_ptr<IPersonContact>;
using PersonContactArray = std::vector<IPersonContactPtr>;

// Group contact base. Concrete variants (IDepartmentGroup, ITeamGroup, IFolderGroup)
// add typed fields via their own CTI sub-tables; Project / Custom have no sub-row.
class SERVICE_EXPORT IGroupContact: public IContact
{
public:
    enum class GroupType {
        Folder     = 0, // 整理夹
        Department = 1, // 部门
        Project    = 2, // 项目组
        Team       = 3, // 临时/职能小组
        Custom     = 4, // 用户自定义
    };
    virtual std::string getGroupName() const = 0;
    virtual GroupType   getGroupType() const = 0;
};
using IGroupContactPtr  = std::shared_ptr<IGroupContact>;
using GroupContactArray = std::vector<IGroupContactPtr>;

// Department-typed group.
class SERVICE_EXPORT IDepartmentGroup: public IGroupContact
{
public:
    virtual std::string getManagerId() const = 0;
    virtual int         getHeadcount() const = 0;
};
using IDepartmentGroupPtr = std::shared_ptr<IDepartmentGroup>;

// Team-typed group.
class SERVICE_EXPORT ITeamGroup: public IGroupContact
{
public:
    virtual std::string getTeamLeadId() const = 0;
    virtual std::string getMission()    const = 0;
};
using ITeamGroupPtr = std::shared_ptr<ITeamGroup>;

// Folder-typed group. Marker interface; presence of the FolderGroup sub-row is
// the only typed state today.
class SERVICE_EXPORT IFolderGroup: public IGroupContact
{
};
using IFolderGroupPtr = std::shared_ptr<IFolderGroup>;

class SERVICE_EXPORT IContactRelation
{
public:
    enum class RelationType {
        Folder        = 0, // 整理夹成员关系
        Department    = 1, // 部门关系
        Reporting     = 2, // 汇报关系
        Project       = 3, // 项目关系
        Mentor        = 4, // 导师关系
        Collaboration = 5, // 协作关系
    };
    virtual ~IContactRelation() = default;

    // Surrogate primary key. May be empty on freshly-created relations; the
    // service assigns a UUID before persisting.
    virtual std::string getRelationId() const = 0;

    virtual std::string getChildId() const = 0;

    virtual std::string getParentId() const = 0;

    virtual RelationType getRelationType() const = 0;
};

using IContactRelationPtr  = std::shared_ptr<IContactRelation>;
using ContactRelationArray = std::vector<IContactRelationPtr>;
}
