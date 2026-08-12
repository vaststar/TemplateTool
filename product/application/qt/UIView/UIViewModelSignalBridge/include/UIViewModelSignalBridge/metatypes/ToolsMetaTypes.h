#pragma once

#include <vector>

#include <QMetaType>

#include <UIViewModelSignalBridge/metatypes/CommonMetaTypes.h>
#include <commonhead/viewmodels/ToolsViewModel/IToolsModel.h>

Q_DECLARE_METATYPE(commonHead::viewModels::model::ToolsTreePtr)
Q_DECLARE_METATYPE(commonHead::viewModels::model::ToolNodeData)
Q_DECLARE_METATYPE(std::vector<commonHead::viewModels::model::ToolNodeData>)
