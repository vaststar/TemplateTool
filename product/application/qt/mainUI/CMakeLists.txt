set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC
    include/Main/Main.h
    include/Main/MainExport.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    include/Main/LoggerDefine.h 
    src/Main.cpp
)
if (WIN32)
    set(ADD_DEFINITIONS Main_DLL)
else()
    set(ADD_DEFINITIONS)
endif()

include(BuildQtModule)
BuildQtModule(
    MODULE_NAME 
        MainUI
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PRIVATE
        UIView
        CoreFramework
        CommonHeadFramework
        AppRunner
        MasterLog
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