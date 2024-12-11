include(BuildModule)

if (WIN32)
    set(ADD_APPRUNNER_DEFINITIONS APPRUNNER_DLL)
else()
    set(ADD_APPRUNNER_DEFINITIONS)
endif()


set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC  #for include and install header
include/AppRunner/AppRunner.h
include/AppRunner/AppRunnerExport.h
)

set(TARGET_SOURCE_FILES_PRIVATE
src/AppRunner.cpp
src/LoggerDefine.h
)

BuildModule(
    MODULE_NAME 
        AppRunner
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PRIVATE
        MasterLog
        CommonHeadFramework
        ImageService
        DataWarehouseSchema
        DataWarehouseService
        ContactService
        MediaService
        NetworkService
        ClientInfoService
        CoreFramework
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    TARGET_DEFINITIONS
        ${ADD_APPRUNNER_DEFINITIONS}
)