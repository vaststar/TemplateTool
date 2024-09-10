set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC
    include/MainWindow/MainWindowController.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    include/LoggerDefine.h
    include/ContactList/Datas.h
    include/ContactList/ContactListItemModel.h
    include/ContactList/ContactListViewController.h
    include/MainWindow/MainWindowManager.h

    src/Datas.cpp
    src/ContactListViewController.cpp
    src/MainWindowController.cpp
    src/ContactListItemModel.cpp
    src/MainWindowManager.cpp
)

set(UIVIEW_QML_TARGET_FILES
    qml/ContactList/ContactList.qml
    qml/MainWindow/AppHeader.qml
    qml/MainWindow/AppMenuBar.qml
    qml/MainWindow/AppFooter.qml
    qml/MainWindow/MainWindowContent.qml
    qml/MainWindow/MainWindow.qml
    qml/MainWindow/MainWindowTabBar.qml
    qml/testUI/testUI.qml
    qml/testUI/testWindow.qml
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
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PRIVATE
        UICore
        Qt6::Gui Qt6::Quick Qt6::Widgets
        UIComponent
        ViewModels
        CommonHeadFramework
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
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