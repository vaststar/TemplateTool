#pragma once

#include <QMetaType>
#include <vector>
#include <commonHead/viewModels/SideBarViewModel/ISideBarViewModel.h>
#include <commonHead/viewModels/MediaCameraViewModel/IMediaCameraViewModel.h>

Q_DECLARE_METATYPE(commonHead::viewModels::model::NavItemData)
Q_DECLARE_METATYPE(commonHead::viewModels::model::PageChangeEvent)
Q_DECLARE_METATYPE(std::vector<commonHead::viewModels::model::NavItemData>)
Q_DECLARE_METATYPE(commonHead::viewModels::model::VideoFrame)

namespace UIVMSignalEmitter {

inline void registerAllViewModelMetaTypes()
{
    qRegisterMetaType<commonHead::viewModels::model::NavItemData>();
    qRegisterMetaType<commonHead::viewModels::model::PageChangeEvent>();
    qRegisterMetaType<std::vector<commonHead::viewModels::model::NavItemData>>();
    qRegisterMetaType<commonHead::viewModels::model::VideoFrame>();
}

} // namespace UIVMSignalEmitter
