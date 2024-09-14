set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC
    AppUIManager/include/AppUIManager.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    LoggerDefine/LoggerDefine.h

    ContactList/include/Datas.h
    ContactList/include/ContactListItemModel.h
    ContactList/include/ContactListViewController.h
    ContactList/src/Datas.cpp
    ContactList/src/ContactListViewController.cpp
    ContactList/src/ContactListItemModel.cpp

    MainWindow/include/MainWindowController.h
    MainWindow/src/MainWindowController.cpp

    AppUIManager/src/AppUIManager.cpp
)

set(UIVIEW_QML_TARGET_FILES
    ContactList/qml/ContactList.qml
    MainWindow/qml/AppHeader.qml
    MainWindow/qml/AppMenuBar.qml
    MainWindow/qml/AppFooter.qml
    MainWindow/qml/MainWindowContent.qml
    MainWindow/qml/MainWindow.qml
    MainWindow/qml/MainWindowTabBar.qml
)

set(UIVIEW_QML_TARGET_RESOURCES
    picture/112.png
)

set(UIVIEW_QML_PUBLIC_BUILD_INTERFACE_FOLDER
    ${CMAKE_CURRENT_SOURCE_DIR}/ContactList/include
    ${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/include
)

include(BuildQtModule)
BuildQtModule(
    MODULE_NAME 
        UIView
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PRIVATE
        UICore
        Qt6::Gui Qt6::Quick Qt6::Widgets
        UIComponent
        ViewModels
        CommonHeadFramework
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        include
    IDE_FOLDER
        UI
    QML_TARGET_URI
        UIView
    QML_TARGET_FILES
        ${UIVIEW_QML_TARGET_FILES}
    QML_TARGET_RESOURCES_DIR
        ../../rescources
    QML_TARGET_RESOURCES
        ${UIVIEW_QML_TARGET_RESOURCES}
    QML_PUBLIC_BUILD_INTERFACE_FOLDER
        ${UIVIEW_QML_PUBLIC_BUILD_INTERFACE_FOLDER}
)