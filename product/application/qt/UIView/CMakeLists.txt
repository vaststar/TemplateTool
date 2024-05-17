
set(UIVIEW_TARGET_SOURCE
    include/LoggerDefine.h
)

set(UIVIEW_BUILD_INTERFACE_FOLDER
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(UIVIEW_INSTALL_PUBLIC_HEADER
    include/ContactList/Datas.h
    include/ContactList/ContactListViewController.h
    include/MainWindow/MainWindowController.h
)

set(UIVIEW_QML_TARGET_FILES
    qml/ContactList/ContactList.qml
    qml/MainWindow/AppHeader.qml
    qml/MainWindow/AppMenuBar.qml
    qml/MainWindow/AppFooter.qml
    qml/MainWindow/MainWindowContent.qml
    qml/MainWindow/MainWindow.qml
    qml/MainWindow/MainWindowTabBar.qml
)

set(UIVIEW_QML_TARGET_SOURCES
    include/ContactList/Datas.h 
    include/ContactList/ContactListViewController.h
    include/ContactList/ContactListItemModel.h
    include/MainWindow/MainWindowController.h
    src/Datas.cpp
    src/ContactListViewController.cpp
    src/MainWindowController.cpp
    src/ContactListItemModel.cpp
)

set(UIVIEW_QML_TARGET_RESOURCES
    picture/112.png
)

set(UIVIEW_QML_PUBLIC_BUILD_INTERFACE_FOLDER
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ContactList
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MainWindow
)

include(BuildQtModule)
BuildQtModule(
MODULE_NAME 
    UIView
TARGET_SOURCE
    ${UIVIEW_TARGET_SOURCE}
TARGET_DEPENDENICES_PRIVATE
    UIComponent
    ViewModels
    CommonHeadFramework
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${UIVIEW_BUILD_INTERFACE_FOLDER}
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        include
INSTALL_PUBLIC_HEADER 
    ${UIVIEW_INSTALL_PUBLIC_HEADER}
IDE_FOLDER
    UI
QML_TARGET_URI
    UIView
QML_TARGET_FILES
    ${UIVIEW_QML_TARGET_FILES}
QML_TARGET_SOURCES
    ${UIVIEW_QML_TARGET_SOURCES}
QML_TARGET_RESOURCES_DIR
    ../../rescources
QML_TARGET_RESOURCES
    ${UIVIEW_QML_TARGET_RESOURCES}
QML_PUBLIC_BUILD_INTERFACE_FOLDER
    ${UIVIEW_QML_PUBLIC_BUILD_INTERFACE_FOLDER}
)