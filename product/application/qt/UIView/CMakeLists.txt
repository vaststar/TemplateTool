include(BuildQtModule)
if (WIN32)
    set(ADD_DEFINITIONS AppUI_DLL) #AppUI_DLL AppUI_LIB
else()
    set(ADD_DEFINITIONS)
endif()

set(MODULE_MODULE_NAME UIView)
set(MODULE_IDE_FOLDER UI)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(MODULE_HEADER_FILES #for include and install header
    AppUIManager/include/AppUIManager.h
    AppUIManager/include/AppUIExport.h
)
set(MODULE_SOURCE_PRIVATE
    LoggerDefine/LoggerDefine.h

    UIViewBase/include/UIViewController.h
    UIViewBase/src/UIViewController.cpp
    UIViewBase/include/UIViewControllerInitializer.h
    UIViewBase/src/UIViewControllerInitializer.cpp

    ViewModelSingalEmitter/RegisterViewModelMetaTypes.h
    ViewModelSingalEmitter/MediaCameraViewModelEmitter.h
    ViewModelSingalEmitter/InvocationViewModelEmitter.h
    ViewModelSingalEmitter/AppUiViewModelEmitter.h
    ViewModelSingalEmitter/MainWindowViewModelEmitter.h
    ViewModelSingalEmitter/StabilityViewModelEmitter.h
    ViewModelSingalEmitter/SideBarViewModelEmitter.h
    ViewModelSingalEmitter/ToolsViewModelEmitter.h
    ViewModelSingalEmitter/SettingsViewModelEmitter.h
    ViewModelSingalEmitter/NetworkProxyViewModelEmitter.h

    PageViews/ContactsPage/include/ContactListItemModel.h
    PageViews/ContactsPage/include/ContactsPageController.h
    PageViews/ContactsPage/src/ContactListItemModel.cpp
    PageViews/ContactsPage/src/ContactsPageController.cpp
    PageViews/HomePage/include/HomePageController.h
    PageViews/HomePage/src/HomePageController.cpp
    PageViews/SettingsPage/include/SettingsPageController.h
    PageViews/SettingsPage/include/SettingsTreeModel.h
    PageViews/SettingsPage/include/AppearanceSettingsController.h
    PageViews/SettingsPage/include/LanguageSettingsController.h
    PageViews/SettingsPage/src/SettingsPageController.cpp
    PageViews/SettingsPage/src/SettingsTreeModel.cpp
    PageViews/SettingsPage/src/AppearanceSettingsController.cpp
    PageViews/SettingsPage/src/LanguageSettingsController.cpp

    PageViews/ToolsPage/common/include/ToolsPageController.h
    PageViews/ToolsPage/common/include/ToolsTreeModel.h
    PageViews/ToolsPage/textprocess/include/Base64ToolController.h
    PageViews/ToolsPage/textprocess/include/JsonToolController.h
    PageViews/ToolsPage/textprocess/include/TimestampToolController.h
    PageViews/ToolsPage/generators/include/UuidToolController.h
    PageViews/ToolsPage/network/include/NetworkProxyController.h
    PageViews/ToolsPage/network/include/ProxyRequestModel.h
    PageViews/ToolsPage/network/include/ProxyRulesManager.h
    PageViews/ToolsPage/common/src/ToolsPageController.cpp
    PageViews/ToolsPage/common/src/ToolsTreeModel.cpp
    PageViews/ToolsPage/textprocess/src/Base64ToolController.cpp
    PageViews/ToolsPage/textprocess/src/JsonToolController.cpp
    PageViews/ToolsPage/textprocess/src/TimestampToolController.cpp
    PageViews/ToolsPage/generators/src/UuidToolController.cpp
    PageViews/ToolsPage/network/src/NetworkProxyController.cpp
    PageViews/ToolsPage/network/src/ProxyRequestModel.cpp
    PageViews/ToolsPage/network/src/ProxyRulesManager.cpp
    PageViews/ToolsPage/screencapture/include/ScreenCaptureController.h
    PageViews/ToolsPage/screencapture/include/ScreenshotService.h
    PageViews/ToolsPage/screencapture/src/ScreenCaptureController.cpp
    PageViews/ToolsPage/screencapture/src/ScreenshotService.cpp

    MainWindowTitleBar/include/MainWindowTitleBarController.h
    MainWindowTitleBar/src/MainWindowTitleBarController.cpp

    MainWindowFootBar/include/MainWindowFootBarController.h
    MainWindowFootBar/src/MainWindowFootBarController.cpp

    MainWindowMenuBar/include/MainWindowMenuBarController.h
    MainWindowMenuBar/src/MainWindowMenuBarController.cpp

    MainWindowSideBar/include/MainWindowSideBarController.h
    MainWindowSideBar/include/NavItemModel.h
    MainWindowSideBar/src/MainWindowSideBarController.cpp
    MainWindowSideBar/src/NavItemModel.cpp

    AppSystemTray/include/AppSystemTrayController.h
    AppSystemTray/src/AppSystemTrayController.cpp

    MediaCameraView/include/MediaCameraViewController.h
    MediaCameraView/src/MediaCameraViewController.cpp

    MainWindow/include/MainWindowController.h
    MainWindow/src/MainWindowController.cpp

    AppUIManager/src/AppUIManager.cpp
    AppUIManager/src/AppUIController.h
    AppUIManager/src/AppUIController.cpp
    AppUIManager/src/UIIPCServerHelper.h
    AppUIManager/src/UIIPCServerHelper.cpp
    AppUIManager/src/UIStabilityMonitor.h
    AppUIManager/src/UIStabilityMonitor.cpp

)
set(MODULE_DEPENDENCIES_PUBLIC
)
set(MODULE_DEPENDENCIES_PRIVATE
    UIPlatformUtils
    UIIPCChannel
    UTComponent
    AppContext
    UIResourceAssetRCCInterface #for asset resource
    UIResourcecTranslation #for translation
    UIManager
    ViewModels
    CommonHeadFramework
    ResourceLoader
    Qt6::Gui Qt6::Quick Qt6::Widgets Qt6::Multimedia Qt6::Network
)

if(WIN32)
    list(APPEND MODULE_DEPENDENCIES_PRIVATE crypt32)
endif()

set(MODULE_BUILD_INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR include)

set(MODULE_QML_TARGET_URI
    ${MODULE_MODULE_NAME}
)
set(MODULE_QML_TARGET_FILES
    PageViews/HomePage/qml/HomePage.qml
    PageViews/ContactsPage/qml/ContactsPage.qml
    PageViews/ContactsPage/qml/ContactListPanel.qml
    PageViews/ContactsPage/qml/ContactDetailPanel.qml
    PageViews/SettingsPage/qml/SettingsPage.qml
    PageViews/SettingsPage/qml/SettingsNavPanel.qml
    PageViews/SettingsPage/qml/AppearanceSettingsPanel.qml
    PageViews/SettingsPage/qml/LanguageSettingsPanel.qml
    PageViews/ToolsPage/common/qml/ToolsPage.qml
    PageViews/ToolsPage/common/qml/ToolsNavPanel.qml
    PageViews/ToolsPage/textprocess/qml/Base64Panel.qml
    PageViews/ToolsPage/textprocess/qml/JsonPanel.qml
    PageViews/ToolsPage/textprocess/qml/TimestampPanel.qml
    PageViews/ToolsPage/generators/qml/UuidPanel.qml
    PageViews/ToolsPage/network/qml/NetworkProxyPanel.qml
    PageViews/ToolsPage/network/qml/NetworkCaptureTab.qml
    PageViews/ToolsPage/network/qml/NetworkRulesTab.qml
    PageViews/ToolsPage/network/qml/NetworkSettingsTab.qml
    PageViews/ToolsPage/network/qml/PatternTestDialog.qml
    PageViews/ToolsPage/network/qml/MockDetailDialog.qml
    PageViews/ToolsPage/screencapture/qml/ScreenCapturePanel.qml
    PageViews/ToolsPage/screencapture/qml/ScreenshotTab.qml
    PageViews/ToolsPage/screencapture/qml/ScreenshotOverlay.qml
    PageViews/ToolsPage/screencapture/qml/RecordingTab.qml
    PageViews/ToolsPage/screencapture/qml/ScreenCaptureSettingsTab.qml
    PageViews/PlaceholderPage/qml/PlaceholderPage.qml
    MainWindowTitleBar/qml/MainWindowTitleBar.qml
    MainWindowMenuBar/qml/MainWindowMenuBar.qml
    MainWindowFootBar/qml/MainWindowFootBar.qml
    MainWindowSideBar/qml/MainWindowSideBar.qml
    MainWindowSideBar/qml/NavItemDelegate.qml
    MediaCameraView/qml/MediaCameraView.qml
    MainWindow/qml/MainWindowContent.qml
    MainWindow/qml/MainWindow.qml
    MainWindow/qml/WindowResizeHandler.qml
    AppSystemTray/qml/AppSystemTray.qml
)
BuildQtModule(
MODULE_NAME ${MODULE_MODULE_NAME}
TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
TARGET_ADD_LINK_LIBRARY_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE  ${MODULE_BUILD_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
TARGET_DEFINITIONS ${ADD_DEFINITIONS}
IDE_FOLDER ${MODULE_IDE_FOLDER}
QML_TARGET_URI ${MODULE_QML_TARGET_URI}
QML_TARGET_FILES ${MODULE_QML_TARGET_FILES}
)
