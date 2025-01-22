set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC
    include/UIManager/UIManagerExport.h
    include/UIManager/TranslatorManager.h
    include/UIManager/UIManagerProvider.h
    include/UIManager/ThemeManager.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/LoggerDefine.h 
    src/TranslatorManager.cpp
    src/UIManagerProvider.cpp
    src/ThemeManager.cpp
)

if (WIN32)
    set(ADD_DEFINITIONS UIManager_DLL)
else()
    set(ADD_DEFINITIONS)
endif()

include(BuildQtModule)
BuildQtModule(
    MODULE_NAME 
        UIManager
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PRIVATE
        ResourceLoader
        CommonHeadFramework
        ViewModels
        MasterLog
        UICore
    TARGET_DEPENDENICES_PUBLIC
        Qt6::Widgets
        Qt6::Qml
        UIDataStruct
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
)