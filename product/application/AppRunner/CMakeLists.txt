include(BuildModule)

if (WIN32)
    set(ADD_APPRUNNER_DEFINITIONS APPRUNNER_DLL APPRUNNER_LIB)
endif()

BuildModule(
    MODULE_NAME 
        AppRunner
    TARGET_SOURCE 
        include/AppRunner/AppRunner.h
        include/AppRunner/AppRunnerExport.h
        src/AppRunner.cpp
    TARGET_PRIVATE_DEPENDENICES
        CommonHeadFramework
        CoreFramework
    TARGET_PUBLIC_DEPENDENICES
        MasterLog
    PUBLIC_BUILD_INTERFACE_FOLDER
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC_INSTALL_INTERFACE_FOLDER
        include
    PRIVATE_BUILD_FOLDER
        src
    TARGET_DEFINITIONS
        ${ADD_APPRUNNER_DEFINITIONS}
    INSTALL_PUBLIC_HEADER 
        include/AppRunner/AppRunner.h
        include/AppRunner/AppRunnerExport.h
)