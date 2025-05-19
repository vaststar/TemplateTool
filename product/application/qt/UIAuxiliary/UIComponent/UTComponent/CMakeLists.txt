set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC
    include/UTComponentExport.h
    UTMenu/MenuItemModel.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    include/LoggerDefine.h
    UTMenu/MenuItemModel.cpp
)

set(MODULE_QML_TARGET_FILES
    UTWindow/UTDialog.qml
    UTWindow/UTWindow.qml
    UTWindow/UTTest.qml
    UTMenu/UTMenu.qml
    UTMenu/UTMenuItem.qml
    UTMenu/CreateMenus.js
    UTButton/UTButton.qml
    UTFocusItem/UTFocusItem.qml
    UTComponentUtil/UTComponentUtil.js
)

set(MODULE_TARGET_SOURCE_HEADER_BASE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(MODULE_QML_PUBLIC_BUILD_INTERFACE_FOLDER
    ${CMAKE_CURRENT_SOURCE_DIR}/UTMenu
)

if (WIN32)
    set(ADD_DEFINITIONS UTCOMPONENT_DLL)
else()
    set(ADD_DEFINITIONS)
endif()

include(BuildQtModule)
BuildQtModule(
    MODULE_NAME 
        UTComponent
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${MODULE_TARGET_SOURCE_HEADER_BASE_DIR}
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PRIVATE
        Qt6::Gui Qt6::Quick
        MasterLog 
        UIDataStruct
        UIComponentBase
    TARGET_DEFINITIONS
        ${ADD_DEFINITIONS}
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    IDE_FOLDER
        UI
    QML_TARGET_URI
        UTComponent
    QML_TARGET_FILES
        ${MODULE_QML_TARGET_FILES}
    QML_PUBLIC_BUILD_INTERFACE_FOLDER
        ${MODULE_QML_PUBLIC_BUILD_INTERFACE_FOLDER}
)