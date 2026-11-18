#include "ContactsPageUtils.h"

#include <QCoreApplication>

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
        return QCoreApplication::translate("ContactsPageUtils", "Contact");
    }
    switch (groupType)
    {
        case commonHead::viewModels::model::GroupType::Folder:     return QCoreApplication::translate("ContactsPageUtils", "Folder");
        case commonHead::viewModels::model::GroupType::Department: return QCoreApplication::translate("ContactsPageUtils", "Department");
        case commonHead::viewModels::model::GroupType::Project:    return QCoreApplication::translate("ContactsPageUtils", "Project group");
        case commonHead::viewModels::model::GroupType::Team:       return QCoreApplication::translate("ContactsPageUtils", "Team");
        case commonHead::viewModels::model::GroupType::Custom:     return QCoreApplication::translate("ContactsPageUtils", "Custom group");
    }
    return QCoreApplication::translate("ContactsPageUtils", "Unknown");
}

QString statusLabel(commonHead::viewModels::model::ContactStatus status)
{
    switch (status)
    {
        case commonHead::viewModels::model::ContactStatus::Active:   return QCoreApplication::translate("ContactsPageUtils", "Active");
        case commonHead::viewModels::model::ContactStatus::Inactive: return QCoreApplication::translate("ContactsPageUtils", "Inactive");
        case commonHead::viewModels::model::ContactStatus::Deleted:  return QCoreApplication::translate("ContactsPageUtils", "Deleted");
        case commonHead::viewModels::model::ContactStatus::Archived: return QCoreApplication::translate("ContactsPageUtils", "Archived");
    }
    return QCoreApplication::translate("ContactsPageUtils", "Unknown");
}

QString genderLabel(commonHead::viewModels::model::Gender gender)
{
    switch (gender)
    {
        case commonHead::viewModels::model::Gender::Unspecified: return QCoreApplication::translate("ContactsPageUtils", "Unspecified");
        case commonHead::viewModels::model::Gender::Male:        return QCoreApplication::translate("ContactsPageUtils", "Male");
        case commonHead::viewModels::model::Gender::Female:      return QCoreApplication::translate("ContactsPageUtils", "Female");
        case commonHead::viewModels::model::Gender::Other:       return QCoreApplication::translate("ContactsPageUtils", "Other");
    }
    return QCoreApplication::translate("ContactsPageUtils", "Unknown");
}

namespace {

QVariantMap toPersonMap(const commonHead::viewModels::model::PersonContactDetail& person)
{
    QVariantMap personMap;
    personMap["firstName"]   = QString::fromStdString(person.firstName);
    personMap["lastName"]    = QString::fromStdString(person.lastName);
    personMap["gender"]      = genderLabel(person.gender);
    personMap["genderValue"] = static_cast<int>(person.gender);
    personMap["phone"]       = QString::fromStdString(person.phone);
    personMap["email"]       = QString::fromStdString(person.email);
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

commonHead::viewModels::model::ContactDetail toContactDetail(
    const QString& id,
    const QVariantMap& fields)
{
    using namespace commonHead::viewModels::model;

    ContactDetail detail;
    detail.id = id.toStdString();
    detail.type = fields.value(QStringLiteral("nodeType")).toInt() == 1
        ? ContactNodeType::Group
        : ContactNodeType::Person;
    detail.groupType = static_cast<GroupType>(
        fields.value(QStringLiteral("groupType"),
                     static_cast<int>(GroupType::Folder)).toInt());

    applyEditableFields(detail, fields);
    return detail;
}

void applyEditableFields(
    commonHead::viewModels::model::ContactDetail& detail,
    const QVariantMap& fields)
{
    using namespace commonHead::viewModels::model;

    if (fields.contains(QStringLiteral("displayName")))
    {
        detail.displayName = fields.value(QStringLiteral("displayName"))
                                 .toString()
                                 .trimmed()
                                 .toStdString();
    }

    if (detail.type != ContactNodeType::Person)
    {
        return;
    }

    PersonContactDetail person = detail.person.value_or(PersonContactDetail{});

    if (fields.contains(QStringLiteral("firstName")))
    {
        person.firstName = fields.value(QStringLiteral("firstName"))
                               .toString()
                               .trimmed()
                               .toStdString();
    }
    if (fields.contains(QStringLiteral("lastName")))
    {
        person.lastName = fields.value(QStringLiteral("lastName"))
                              .toString()
                              .trimmed()
                              .toStdString();
    }
    if (fields.contains(QStringLiteral("gender")))
    {
        const int genderValue = fields.value(QStringLiteral("gender")).toInt();
        if (genderValue >= static_cast<int>(Gender::Unspecified)
            && genderValue <= static_cast<int>(Gender::Other))
        {
            person.gender = static_cast<Gender>(genderValue);
        }
    }
    if (fields.contains(QStringLiteral("phone")))
    {
        person.phone = fields.value(QStringLiteral("phone"))
                           .toString()
                           .trimmed()
                           .toStdString();
    }
    if (fields.contains(QStringLiteral("email")))
    {
        person.email = fields.value(QStringLiteral("email"))
                           .toString()
                           .trimmed()
                           .toStdString();
    }

    detail.person = person;
}

} // namespace ContactsPage::Utils
