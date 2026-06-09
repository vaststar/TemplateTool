#pragma once

#include <QMetaType>
#include <string>
#include <vector>
#include <commonHead/viewModels/SideBarViewModel/ISideBarViewModel.h>
#include <commonHead/viewModels/MediaCameraViewModel/IMediaCameraViewModel.h>
#include <commonHead/viewModels/ToolsViewModel/IToolsModel.h>
#include <commonHead/viewModels/SettingsViewModel/ISettingsModel.h>
#include <commonHead/viewModels/CameraDirectoryViewModel/ICameraDirectoryTreeModel.h>
#include <commonHead/viewModels/ContactListViewModel/IContactListModel.h>

Q_DECLARE_METATYPE(commonHead::viewModels::model::NavItemData)
Q_DECLARE_METATYPE(commonHead::viewModels::model::PageChangeEvent)
Q_DECLARE_METATYPE(std::vector<commonHead::viewModels::model::NavItemData>)
Q_DECLARE_METATYPE(commonHead::viewModels::model::SubMenuItem)
Q_DECLARE_METATYPE(std::vector<commonHead::viewModels::model::SubMenuItem>)
Q_DECLARE_METATYPE(commonHead::viewModels::model::VideoFrame)
Q_DECLARE_METATYPE(commonHead::viewModels::model::ToolsTreePtr)
Q_DECLARE_METATYPE(commonHead::viewModels::model::SettingsTreePtr)
Q_DECLARE_METATYPE(commonHead::viewModels::model::SettingsNodeData)
Q_DECLARE_METATYPE(std::vector<commonHead::viewModels::model::SettingsNodeData>)
Q_DECLARE_METATYPE(commonHead::viewModels::model::ToolNodeData)
Q_DECLARE_METATYPE(std::vector<commonHead::viewModels::model::ToolNodeData>)
Q_DECLARE_METATYPE(commonHead::viewModels::model::CameraDirectoryNodeData)
Q_DECLARE_METATYPE(std::vector<commonHead::viewModels::model::CameraDirectoryNodeData>)
Q_DECLARE_METATYPE(commonHead::viewModels::model::CameraDirectoryRelationData)
Q_DECLARE_METATYPE(std::vector<commonHead::viewModels::model::CameraDirectoryRelationData>)
Q_DECLARE_METATYPE(commonHead::viewModels::model::CameraDirectoryLoadError)
Q_DECLARE_METATYPE(commonHead::viewModels::model::ContactNodeData)
Q_DECLARE_METATYPE(std::vector<commonHead::viewModels::model::ContactNodeData>)
Q_DECLARE_METATYPE(commonHead::viewModels::model::ContactRelationData)
Q_DECLARE_METATYPE(std::vector<commonHead::viewModels::model::ContactRelationData>)
Q_DECLARE_METATYPE(commonHead::viewModels::model::ContactDirectoryLoadError)
Q_DECLARE_METATYPE(std::vector<std::string>)

namespace UIVMSignalEmitter {

inline void registerAllViewModelMetaTypes()
{
    qRegisterMetaType<commonHead::viewModels::model::NavItemData>();
    qRegisterMetaType<commonHead::viewModels::model::PageChangeEvent>();
    qRegisterMetaType<std::vector<commonHead::viewModels::model::NavItemData>>();
    qRegisterMetaType<commonHead::viewModels::model::SubMenuItem>();
    qRegisterMetaType<std::vector<commonHead::viewModels::model::SubMenuItem>>();
    qRegisterMetaType<commonHead::viewModels::model::VideoFrame>();
    qRegisterMetaType<commonHead::viewModels::model::ToolsTreePtr>();
    qRegisterMetaType<commonHead::viewModels::model::SettingsTreePtr>();
    qRegisterMetaType<commonHead::viewModels::model::SettingsNodeData>();
    qRegisterMetaType<std::vector<commonHead::viewModels::model::SettingsNodeData>>();
    qRegisterMetaType<commonHead::viewModels::model::ToolNodeData>();
    qRegisterMetaType<std::vector<commonHead::viewModels::model::ToolNodeData>>();
    qRegisterMetaType<commonHead::viewModels::model::CameraDirectoryNodeData>();
    qRegisterMetaType<std::vector<commonHead::viewModels::model::CameraDirectoryNodeData>>();
    qRegisterMetaType<commonHead::viewModels::model::CameraDirectoryRelationData>();
    qRegisterMetaType<std::vector<commonHead::viewModels::model::CameraDirectoryRelationData>>();
    qRegisterMetaType<commonHead::viewModels::model::CameraDirectoryLoadError>();
    qRegisterMetaType<commonHead::viewModels::model::ContactNodeData>();
    qRegisterMetaType<std::vector<commonHead::viewModels::model::ContactNodeData>>();
    qRegisterMetaType<commonHead::viewModels::model::ContactRelationData>();
    qRegisterMetaType<std::vector<commonHead::viewModels::model::ContactRelationData>>();
    qRegisterMetaType<commonHead::viewModels::model::ContactDirectoryLoadError>();
    qRegisterMetaType<std::vector<std::string>>();
}

} // namespace UIVMSignalEmitter
