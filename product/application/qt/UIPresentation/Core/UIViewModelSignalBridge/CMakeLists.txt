include(BuildQtModule)

if (WIN32)
    set(ADD_DEFINITIONS UIViewModelSignalBridge_DLL)
else()
    set(ADD_DEFINITIONS)
endif()

set(MODULE_MODULE_NAME UIViewModelSignalBridge)
set(MODULE_IDE_FOLDER UI/UIPresentation/Core)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/UIViewModelSignalBridge/UIViewModelSignalBridgeExport.h
    include/UIViewModelSignalBridge/RegisterViewModelMetaTypes.h
    include/UIViewModelSignalBridge/metatypes/CommonMetaTypes.h
    include/UIViewModelSignalBridge/metatypes/CameraDirectoryMetaTypes.h
    include/UIViewModelSignalBridge/metatypes/ContactListMetaTypes.h
    include/UIViewModelSignalBridge/metatypes/MediaCameraMetaTypes.h
    include/UIViewModelSignalBridge/metatypes/RecordingMetaTypes.h
    include/UIViewModelSignalBridge/metatypes/ScreenshotMetaTypes.h
    include/UIViewModelSignalBridge/metatypes/SettingsMetaTypes.h
    include/UIViewModelSignalBridge/metatypes/SideBarMetaTypes.h
    include/UIViewModelSignalBridge/metatypes/ToolsMetaTypes.h
    include/UIViewModelSignalBridge/metatypes/UpgradeMetaTypes.h
    include/UIViewModelSignalBridge/emitters/AppUIViewModelEmitter.h
    include/UIViewModelSignalBridge/emitters/CameraDirectoryViewModelEmitter.h
    include/UIViewModelSignalBridge/emitters/ContactListViewModelEmitter.h
    include/UIViewModelSignalBridge/emitters/InvocationViewModelEmitter.h
    include/UIViewModelSignalBridge/emitters/MainWindowViewModelEmitter.h
    include/UIViewModelSignalBridge/emitters/MediaCameraViewModelEmitter.h
    include/UIViewModelSignalBridge/emitters/MiniAppListViewModelEmitter.h
    include/UIViewModelSignalBridge/emitters/MiniAppRuntimeViewModelEmitter.h
    include/UIViewModelSignalBridge/emitters/NetworkProxyViewModelEmitter.h
    include/UIViewModelSignalBridge/emitters/RecordingViewModelEmitter.h
    include/UIViewModelSignalBridge/emitters/ScreenshotViewModelEmitter.h
    include/UIViewModelSignalBridge/emitters/SettingsViewModelEmitter.h
    include/UIViewModelSignalBridge/emitters/SideBarViewModelEmitter.h
    include/UIViewModelSignalBridge/emitters/StabilityViewModelEmitter.h
    include/UIViewModelSignalBridge/emitters/ToolsViewModelEmitter.h
    include/UIViewModelSignalBridge/emitters/UpgradeViewModelEmitter.h
)

set(MODULE_SOURCE_PRIVATE
    src/RegisterViewModelMetaTypes.cpp
)

set(MODULE_DEPENDENCIES_PUBLIC
    AppUIViewModelApi
    CameraDirectoryViewModelApi
    ContactListViewModelApi
    InvocationViewModelApi
    MainWindowViewModelApi
    MediaCameraViewModelApi
    MiniAppListViewModelApi
    MiniAppRuntimeViewModelApi
    NetworkProxyViewModelApi
    RecordingViewModelApi
    ScreenshotViewModelApi
    SettingsViewModelApi
    SideBarViewModelApi
    StabilityViewModelApi
    ToolsViewModelApi
    UpgradeViewModelApi
    Qt6::Core
)

set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)

BuildQtModule(
    MODULE_NAME ${MODULE_MODULE_NAME}
    TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
    TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
    TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${MODULE_BUILD_INTERFACE_DIR}
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
    TARGET_DEFINITIONS ${ADD_DEFINITIONS}
    IDE_FOLDER ${MODULE_IDE_FOLDER}
)
