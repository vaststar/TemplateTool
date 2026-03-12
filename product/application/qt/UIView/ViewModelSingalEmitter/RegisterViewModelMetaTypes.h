#pragma once

#include <QMetaType>
#include <vector>
#include <commonHead/viewModels/SideBarViewModel/ISideBarViewModel.h>
#include <commonHead/viewModels/MediaCameraViewModel/IMediaCameraViewModel.h>
#include <commonHead/viewModels/ToolsViewModel/IToolsModel.h>
#include <commonHead/viewModels/SettingsViewModel/ISettingsModel.h>

Q_DECLARE_METATYPE(commonHead::viewModels::model::NavItemData)
Q_DECLARE_METATYPE(commonHead::viewModels::model::PageChangeEvent)
Q_DECLARE_METATYPE(std::vector<commonHead::viewModels::model::NavItemData>)
Q_DECLARE_METATYPE(commonHead::viewModels::model::VideoFrame)
Q_DECLARE_METATYPE(commonHead::viewModels::model::ToolsTreePtr)
Q_DECLARE_METATYPE(commonHead::viewModels::model::SettingsTreePtr)

namespace UIVMSignalEmitter {

inline void registerAllViewModelMetaTypes()
{
    qRegisterMetaType<commonHead::viewModels::model::NavItemData>();
    qRegisterMetaType<commonHead::viewModels::model::PageChangeEvent>();
    qRegisterMetaType<std::vector<commonHead::viewModels::model::NavItemData>>();
    qRegisterMetaType<commonHead::viewModels::model::VideoFrame>();
    qRegisterMetaType<commonHead::viewModels::model::ToolsTreePtr>();
    qRegisterMetaType<commonHead::viewModels::model::SettingsTreePtr>();
}

} // namespace UIVMSignalEmitter
