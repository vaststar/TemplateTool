#pragma once

#include <vector>

#include <QMetaType>

#include <UIViewModelSignalBridge/metatypes/CommonMetaTypes.h>
#include <commonhead/viewmodels/CameraDirectoryViewModel/ICameraDirectoryTreeModel.h>

Q_DECLARE_METATYPE(commonHead::viewModels::model::CameraDirectoryNodeData)
Q_DECLARE_METATYPE(std::vector<commonHead::viewModels::model::CameraDirectoryNodeData>)
Q_DECLARE_METATYPE(commonHead::viewModels::model::CameraDirectoryRelationData)
Q_DECLARE_METATYPE(std::vector<commonHead::viewModels::model::CameraDirectoryRelationData>)
Q_DECLARE_METATYPE(commonHead::viewModels::model::CameraDirectoryLoadError)
