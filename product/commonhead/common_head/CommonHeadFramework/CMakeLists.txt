include(BuildModule)

BuildModule(
    MODULE_NAME 
        CommonHeadFramework
    TARGET_SOURCE 
        include/CommonHeadFramework/ICommonHeadFramework.h
        include/CommonHeadFramework/CommonHeadFramework.h
        src/CommonHeadFramework.cpp
    TARGET_PRIVATE_DEPENDENICES
        MasterLog
        # CoreFramework
    TARGET_PUBLIC_DEPENDENICES
        CommonHeadCommonFile
    PUBLIC_BUILD_INTERFACE_FOLDER
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC_INSTALL_INTERFACE_FOLDER
        include
    PRIVATE_BUILD_FOLDER
        src
    TARGET_DEFINITIONS
        ${ADD_COMMONHEAD_DEFINITIONS}
    INSTALL_PUBLIC_HEADER 
        include/CommonHeadFramework/ICommonHeadFramework.h  
    IDE_FOLDER
        CommonHead
)