#pragma once

#include <QString>
#include <QVariantMap>

#include <commonhead/viewmodels/ContactListViewModel/IContactListModel.h>

namespace ContactsPage::Utils {

QString kindKey   (commonHead::viewModels::model::ContactNodeType nodeType,
                   commonHead::viewModels::model::GroupType       groupType);
QString kindLabel (commonHead::viewModels::model::ContactNodeType nodeType,
                   commonHead::viewModels::model::GroupType       groupType);
QString statusLabel(commonHead::viewModels::model::ContactStatus status);
QString genderLabel(commonHead::viewModels::model::Gender gender);

// model -> QML: flattens a ContactDetail into a QVariantMap for the detail panel
// and the edit dialog.
QVariantMap toVariantMap(const commonHead::viewModels::model::ContactDetail& detail);

// Creates a new ContactDetail from dialog fields. id may be empty for new contacts.
commonHead::viewModels::model::ContactDetail toContactDetail(
    const QString& id,
    const QVariantMap& fields);

// Applies only user-editable dialog fields to an existing ContactDetail.
// Type, groupType, status and group-specific details are preserved.
void applyEditableFields(
    commonHead::viewModels::model::ContactDetail& detail,
    const QVariantMap& fields);

} // namespace ContactsPage::Utils
