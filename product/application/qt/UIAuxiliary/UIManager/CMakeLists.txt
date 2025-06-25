set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC
    include/UIManager/UIManagerExport.h
    include/UIManager/ITranslatorManager.h
    include/UIManager/IUIManagerProvider.h
    include/UIManager/IThemeManager.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/LoggerDefine.h 
    src/TranslatorManager.h
    src/TranslatorManager.cpp
    src/UIManagerProvider.h
    src/UIManagerProvider.cpp
    src/ThemeManager.h
    src/ThemeManager.cpp
)

if (WIN32)
    set(ADD_DEFINITIONS UIManager_DLL) #UIManager_DLL UIManager_LIB
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
        UICore
        ResourceLoader
        CommonHeadFramework
        ViewModels
        MasterLog
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