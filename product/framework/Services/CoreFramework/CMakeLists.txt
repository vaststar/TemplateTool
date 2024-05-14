include(BuildModule)

BuildModule(
    MODULE_NAME 
        CoreFramework
    TARGET_SOURCE 
        include/CoreFramework/IService.h
        include/CoreFramework/ICoreFramework.h
        include/CoreFramework/ICoreFramework.h
        include/CoreFramework/CoreFramework.h
        include/CoreFramework/ServiceAccessor.h
        include/CoreFramework/ICoreFrameworkCallback.h
        src/CoreFramework.cpp
    TARGET_PRIVATE_DEPENDENICES
        ThreadPool
        
        
    TARGET_PUBLIC_DEPENDENICES
        MasterLog
        ServiceCommonFile
        NotificationHelper
        
    PUBLIC_BUILD_INTERFACE_FOLDER
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC_INSTALL_INTERFACE_FOLDER
        include
    PRIVATE_BUILD_FOLDER
        src
    TARGET_DEFINITIONS
        ${ADD_SERVICE_DEFINITIONS}
    INSTALL_PUBLIC_HEADER 
        include/CoreFramework/ICoreFramework.h  
        include/CoreFramework/IService.h
        include/CoreFramework/ServiceAccessor.h
    IDE_FOLDER
    framework/Services
)