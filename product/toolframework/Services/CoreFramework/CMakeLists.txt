include(BuildService)

BuildService(
    MODULE_NAME 
        CoreFramework
    TARGET_SOURCE 
        include/IService.h
        include/ICoreFramework.h
        include/CoreFramework.h
        src/CoreFramework.cpp
    TARGET_PRIVATE_DEPENDENICES
        MasterLog
    TARGET_PUBLIC_DEPENDENICES
        ServiceCommonFile
    INSTALL_PUBLIC_HEADER 
        include/ICoreFramework.h  
        include/IService.h
)