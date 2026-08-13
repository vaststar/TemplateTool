#include <UIViewModelSignalBridge/RegisterViewModelMetaTypes.h>

#include <mutex>
#include <string>
#include <vector>

#include <QMetaType>

#include <UIViewModelSignalBridge/metatypes/CameraDirectoryMetaTypes.h>
#include <UIViewModelSignalBridge/metatypes/ContactListMetaTypes.h>
#include <UIViewModelSignalBridge/metatypes/MediaCameraMetaTypes.h>
#include <UIViewModelSignalBridge/metatypes/RecordingMetaTypes.h>
#include <UIViewModelSignalBridge/metatypes/ScreenshotMetaTypes.h>
#include <UIViewModelSignalBridge/metatypes/SettingsMetaTypes.h>
#include <UIViewModelSignalBridge/metatypes/SideBarMetaTypes.h>
#include <UIViewModelSignalBridge/metatypes/ToolsMetaTypes.h>
#include <UIViewModelSignalBridge/metatypes/UpgradeMetaTypes.h>

namespace UIViewModelSignalBridge {

void registerAllViewModelMetaTypes()
{
    static std::once_flag registerFlag;
    std::call_once(registerFlag, [] {
        qRegisterMetaType<commonHead::viewModels::model::NavItemData>();
        qRegisterMetaType<std::vector<commonHead::viewModels::model::NavItemData>>();
        qRegisterMetaType<commonHead::viewModels::model::PageId>();
        qRegisterMetaType<std::vector<commonHead::viewModels::model::PageId>>();
        qRegisterMetaType<commonHead::viewModels::model::SubMenuItem>();
        qRegisterMetaType<std::vector<commonHead::viewModels::model::SubMenuItem>>();
        qRegisterMetaType<commonHead::viewModels::model::MenuActionId>();

        qRegisterMetaType<commonHead::viewModels::model::VideoFrame>();

        qRegisterMetaType<commonHead::viewModels::model::ToolsTreePtr>();
        qRegisterMetaType<commonHead::viewModels::model::ToolNodeData>();
        qRegisterMetaType<std::vector<commonHead::viewModels::model::ToolNodeData>>();

        qRegisterMetaType<commonHead::viewModels::model::SettingsTreePtr>();
        qRegisterMetaType<commonHead::viewModels::model::SettingsNodeData>();
        qRegisterMetaType<std::vector<commonHead::viewModels::model::SettingsNodeData>>();

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

        qRegisterMetaType<commonHead::viewModels::model::RecordingSettings>();
        qRegisterMetaType<commonHead::viewModels::model::ScreenshotSettings>();
        qRegisterMetaType<commonHead::viewModels::model::UpgradeViewState>();

        qRegisterMetaType<std::vector<std::string>>();
    });
}

} // namespace UIViewModelSignalBridge
