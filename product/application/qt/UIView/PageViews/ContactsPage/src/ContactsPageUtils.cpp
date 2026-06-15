#include "ContactsPageUtils.h"

namespace ContactsPage::Utils {

QString kindKey(commonHead::viewModels::model::ContactNodeType nodeType,
                commonHead::viewModels::model::GroupType groupType)
{
    if (nodeType == commonHead::viewModels::model::ContactNodeType::Person)
    {
        return QStringLiteral("person");
    }
    switch (groupType)
    {
        case commonHead::viewModels::model::GroupType::Folder:     return QStringLiteral("folder");
        case commonHead::viewModels::model::GroupType::Department: return QStringLiteral("department");
        case commonHead::viewModels::model::GroupType::Project:    return QStringLiteral("project");
        case commonHead::viewModels::model::GroupType::Team:       return QStringLiteral("team");
        case commonHead::viewModels::model::GroupType::Custom:     return QStringLiteral("custom");
    }
    return QStringLiteral("unknown");
}

QString kindLabel(commonHead::viewModels::model::ContactNodeType nodeType,
                  commonHead::viewModels::model::GroupType groupType)
{
    if (nodeType == commonHead::viewModels::model::ContactNodeType::Person)
    {
        return QStringLiteral("联系人");
    }
    switch (groupType)
    {
        case commonHead::viewModels::model::GroupType::Folder:     return QStringLiteral("整理夹");
        case commonHead::viewModels::model::GroupType::Department: return QStringLiteral("部门");
        case commonHead::viewModels::model::GroupType::Project:    return QStringLiteral("项目组");
        case commonHead::viewModels::model::GroupType::Team:       return QStringLiteral("团队");
        case commonHead::viewModels::model::GroupType::Custom:     return QStringLiteral("自定义分组");
    }
    return QStringLiteral("未知");
}

QString statusLabel(commonHead::viewModels::model::ContactStatus status)
{
    switch (status)
    {
        case commonHead::viewModels::model::ContactStatus::Active:   return QStringLiteral("在用");
        case commonHead::viewModels::model::ContactStatus::Inactive: return QStringLiteral("停用");
        case commonHead::viewModels::model::ContactStatus::Deleted:  return QStringLiteral("已删除");
        case commonHead::viewModels::model::ContactStatus::Archived: return QStringLiteral("已归档");
    }
    return QStringLiteral("未知");
}

QString genderLabel(commonHead::viewModels::model::Gender gender)
{
    switch (gender)
    {
        case commonHead::viewModels::model::Gender::Unspecified: return QStringLiteral("未指定");
        case commonHead::viewModels::model::Gender::Male:        return QStringLiteral("男");
        case commonHead::viewModels::model::Gender::Female:      return QStringLiteral("女");
        case commonHead::viewModels::model::Gender::Other:       return QStringLiteral("其他");
    }
    return QStringLiteral("未知");
}

namespace {

QVariantMap toPersonMap(const commonHead::viewModels::model::PersonContactDetail& person)
{
    QVariantMap personMap;
    personMap["firstName"] = QString::fromStdString(person.firstName);
    personMap["lastName"]  = QString::fromStdString(person.lastName);
    personMap["gender"]    = genderLabel(person.gender);
    personMap["phone"]     = QString::fromStdString(person.phone);
    personMap["email"]     = QString::fromStdString(person.email);
    return personMap;
}

QVariantMap toDepartmentMap(const commonHead::viewModels::model::DepartmentGroupDetail& department)
{
    QVariantMap departmentMap;
    departmentMap["managerId"]   = QString::fromStdString(department.managerId);
    departmentMap["managerName"] = QString::fromStdString(department.managerDisplayName);
    departmentMap["headcount"]   = department.headcount;
    return departmentMap;
}

QVariantMap toTeamMap(const commonHead::viewModels::model::TeamGroupDetail& team)
{
    QVariantMap teamMap;
    teamMap["teamLeadId"]   = QString::fromStdString(team.teamLeadId);
    teamMap["teamLeadName"] = QString::fromStdString(team.teamLeadDisplayName);
    teamMap["mission"]      = QString::fromStdString(team.mission);
    return teamMap;
}

} // namespace

QVariantMap toVariantMap(const commonHead::viewModels::model::ContactDetail& detail)
{
    QVariantMap result;
    result["id"]        = QString::fromStdString(detail.id);
    result["name"]      = QString::fromStdString(detail.displayName);
    result["kind"]      = kindKey(detail.type, detail.groupType);
    result["kindLabel"] = kindLabel(detail.type, detail.groupType);
    result["status"]    = statusLabel(detail.status);

    if (detail.person)
    {
        result["person"]     = toPersonMap(*detail.person);
    }
    if (detail.department)
    {
        result["department"] = toDepartmentMap(*detail.department);
    }
    if (detail.team)
    {
        result["team"]       = toTeamMap(*detail.team);
    }
    return result;
}

commonHead::viewModels::model::ContactNodeData toNodeData(const QString& id, const QVariantMap& fields)
{
    using namespace commonHead::viewModels::model;
    ContactNodeData data;
    data.id          = id.toStdString();
    data.displayName = fields.value(QStringLiteral("displayName")).toString().toStdString();
    data.type        = fields.value(QStringLiteral("nodeType")).toInt() == 1
                           ? ContactNodeType::Group
                           : ContactNodeType::Person;
    data.groupType   = static_cast<GroupType>(fields.value(QStringLiteral("groupType"),
                                               static_cast<int>(GroupType::Folder)).toInt());
    return data;
}

} // namespace ContactsPage::Utils
