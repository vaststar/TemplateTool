#pragma once

#include <vector>

#include <QMetaType>

#include <UIViewModelSignalBridge/metatypes/CommonMetaTypes.h>
#include <commonhead/viewmodels/ContactListViewModel/IContactListModel.h>

Q_DECLARE_METATYPE(commonHead::viewModels::model::ContactNodeData)
Q_DECLARE_METATYPE(std::vector<commonHead::viewModels::model::ContactNodeData>)
Q_DECLARE_METATYPE(commonHead::viewModels::model::ContactRelationData)
Q_DECLARE_METATYPE(std::vector<commonHead::viewModels::model::ContactRelationData>)
Q_DECLARE_METATYPE(commonHead::viewModels::model::ContactDirectoryLoadError)
