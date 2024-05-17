
set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC
    include/ClientGlobal/ClientGlobalExport.h
    include/ClientGlobal/ClientGlobal.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    include/ClientGlobal/LoggerDefine.h
    src/ClientGlobal.cpp
)



include(BuildQtModule)
BuildQtModule(
    MODULE_NAME 
        ClientGlobal
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PRIVATE
        Qt6::Gui Qt6::Quick 
        CommonHeadFramework
    TARGET_DEPENDENICES_PUBLIC
        MasterLog ThreadPool
    TARGET_DEFINITIONS
        CLIENTGLOBAL_DLL
        CLIENTGLOBAL_LIB
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    IDE_FOLDER
        UI
)