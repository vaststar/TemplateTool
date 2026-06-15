#pragma once

#include <QString>
#include <QVariantMap>

#include <commonHead/viewModels/ContactListViewModel/IContactListModel.h>

namespace ContactsPage::Utils {

QString kindKey   (commonHead::viewModels::model::ContactNodeType nodeType,
                   commonHead::viewModels::model::GroupType       groupType);
QString kindLabel (commonHead::viewModels::model::ContactNodeType nodeType,
                   commonHead::viewModels::model::GroupType       groupType);
QString statusLabel(commonHead::viewModels::model::ContactStatus status);
QString genderLabel(commonHead::viewModels::model::Gender gender);

// model -> QML: flattens a ContactDetail into a QVariantMap for the detail panel.
QVariantMap toVariantMap(const commonHead::viewModels::model::ContactDetail& detail);

// QML -> model: inverse of toVariantMap for the editable subset. Builds a
// ContactNodeData from {displayName, nodeType, groupType}; id may be empty for new nodes.
commonHead::viewModels::model::ContactNodeData toNodeData(const QString& id,
                                                          const QVariantMap& fields);

} // namespace ContactsPage::Utils
