#pragma once

#include <QString>
#include <QVariantMap>

#include <commonHead/viewModels/ContactListViewModel/IContactListModel.h>

namespace ContactsPage::DetailMapper {

QString kindKey   (commonHead::viewModels::model::ContactNodeType nodeType,
                   commonHead::viewModels::model::GroupType       groupType);
QString kindLabel (commonHead::viewModels::model::ContactNodeType nodeType,
                   commonHead::viewModels::model::GroupType       groupType);
QString statusLabel(commonHead::viewModels::model::ContactStatus status);
QString genderLabel(commonHead::viewModels::model::Gender gender);

QVariantMap toVariantMap(const commonHead::viewModels::model::ContactDetail& detail);

} // namespace ContactsPage::DetailMapper
