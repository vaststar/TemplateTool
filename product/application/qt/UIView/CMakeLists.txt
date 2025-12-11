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

    ViewModelSingalEmitter/MediaCameraViewModelEmitter.h

    ContactList/include/ContactListItemModel.h
    ContactList/include/ContactListViewController.h
    ContactList/src/ContactListViewController.cpp
    ContactList/src/ContactListItemModel.cpp

    MainWindowTitleBar/include/MainWindowTitleBarController.h
    MainWindowTitleBar/src/MainWindowTitleBarController.cpp
    
    MainWindowFootBar/include/MainWindowFootBarController.h
    MainWindowFootBar/src/MainWindowFootBarController.cpp

    MainWindowMenuBar/include/MainWindowMenuBarController.h
    MainWindowMenuBar/src/MainWindowMenuBarController.cpp

    MainWindowSideBar/include/MainWindowSideBarController.h
    MainWindowSideBar/src/MainWindowSideBarController.cpp

    AppSystemTray/include/AppSystemTrayController.h
    AppSystemTray/src/AppSystemTrayController.cpp

    MediaCameraView/include/MediaCameraViewController.h
    MediaCameraView/src/MediaCameraViewController.cpp

    MainWindow/include/MainWindowController.h
    MainWindow/src/MainWindowController.cpp
    
    AppUIManager/src/AppUIManager.cpp
    AppUIManager/src/AppUIController.h
    AppUIManager/src/AppUIController.cpp
    
)
set(MODULE_DEPENDENCIES_PUBLIC
)
set(MODULE_DEPENDENCIES_PRIVATE
    UIPlatformUtils
    UTComponent
    AppContext
    UIResourceAssetRCCInterface #for asset resource
    UIResourcecTranslation #for translation
    UIManager
    ViewModels
    CommonHeadFramework
    ResourceLoader
    Qt6::Gui Qt6::Quick Qt6::Widgets Qt6::Multimedia
)

set(MODULE_BUILD_INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR include)

set(MODULE_QML_TARGET_URI
    ${MODULE_MODULE_NAME}
)
set(MODULE_QML_TARGET_FILES
    ContactList/qml/ContactList.qml
    MainWindowTitleBar/qml/MainWindowTitleBar.qml
    MainWindowTitleBar/qml/WindowAnimator.qml
    MainWindowMenuBar/qml/MainWindowMenuBar.qml
    MainWindowFootBar/qml/MainWindowFootBar.qml
    MainWindowSideBar/qml/MainWindowSideBar.qml
    MediaCameraView/qml/MediaCameraView.qml
    MainWindow/qml/MainWindowContent.qml
    MainWindow/qml/MainWindow.qml
    AppSystemTray/qml/AppSystemTray.qml
)
set(MODULE_QML_PUBLIC_BUILD_INTERFACE_FOLDER
    ${CMAKE_CURRENT_SOURCE_DIR}/UIViewBase/include
    ${CMAKE_CURRENT_SOURCE_DIR}/ContactList/include
    ${CMAKE_CURRENT_SOURCE_DIR}/MainWindowTitleBar/include
    ${CMAKE_CURRENT_SOURCE_DIR}/MainWindowMenuBar/include
    ${CMAKE_CURRENT_SOURCE_DIR}/MainWindowFootBar/include
    ${CMAKE_CURRENT_SOURCE_DIR}/MainWindowSideBar/include
    ${CMAKE_CURRENT_SOURCE_DIR}/AppSystemTray/include
    ${CMAKE_CURRENT_SOURCE_DIR}/MediaCameraView/include
    ${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/include
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
QML_PUBLIC_BUILD_INTERFACE_FOLDER ${MODULE_QML_PUBLIC_BUILD_INTERFACE_FOLDER}
)
