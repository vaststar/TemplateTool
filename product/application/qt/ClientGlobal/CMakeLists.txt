include(BuildQtModule)
BuildQtModule(
MODULE_NAME 
    ClientGlobal
TARGET_SOURCE
include/ClientGlobal/LoggerDefine.h
    include/ClientGlobal/ClientGlobalExport.h
    include/ClientGlobal/ClientGlobal.h
    src/ClientGlobal.cpp
TARGET_PRIVATE_DEPENDENICES
    Qt6::Gui Qt6::Quick 
    CommonHeadFramework
TARGET_PUBLIC_DEPENDENICES
    MasterLog ThreadPool
TARGET_DEFINITIONS
    CLIENTGLOBAL_DLL
    CLIENTGLOBAL_LIB
PUBLIC_BUILD_INTERFACE_FOLDER
    ${CMAKE_CURRENT_SOURCE_DIR}/include
PUBLIC_INSTALL_INTERFACE_FOLDER
    include
INSTALL_PUBLIC_HEADER 
    include/ClientGlobal/ClientGlobalExport.h
    include/ClientGlobal/ClientGlobal.h
IDE_FOLDER
    UI
)