include(BuildModule)

set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC  #for include and install header
    include/ucf/Services/ClientInfoService/IClientInfoServiceCallback.h
    include/ucf/Services/ClientInfoService/IClientInfoService.h
    include/ucf/Services/ClientInfoService/ClientInfoModel.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/ClientInfoServiceLogger.h
    src/ClientInfoService.h
    src/ClientInfoService.cpp
    src/ClientInfoManager.h
    src/ClientInfoManager.cpp
)


BuildModule(
    MODULE_NAME 
        ClientInfoService
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PRIVATE
        MasterLog
        CoreFramework
        TimeUtils
        OSUtils
        StringUtils
        magic_enum
    TARGET_DEPENDENICES_PUBLIC
        ServiceExportMacro
        ServiceDeclaration
        NotificationHelper
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    TARGET_DEFINITIONS
        ${ADD_SERVICE_DEFINITIONS}
    IDE_FOLDER
        framework/Services
)