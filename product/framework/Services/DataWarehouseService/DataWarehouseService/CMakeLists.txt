include(BuildModule)

set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC  #for include and install header
    include/ucf/Services/DataWarehouseService/DatabaseModelDeclare.h
    include/ucf/Services/DataWarehouseService/IDataWarehouseService.h
    include/ucf/Services/DataWarehouseService/DataBaseConfig.h
    include/ucf/Services/DataWarehouseService/DataBaseTableModel.h
    include/ucf/Services/DataWarehouseService/DataBaseDataValue.h
    include/ucf/Services/DataWarehouseService/DatabaseDataRecord.h
    include/ucf/Services/DataWarehouseService/IDataWarehouseServiceCallback.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/DataWarehouseServiceLogger.h
    src/DataBaseTableModel.cpp
    src/DataBaseDataValue.cpp
    src/DatabaseDataRecord.cpp
    src/DataBaseConfig.cpp
    src/DataWarehouseService.h
    src/DataWarehouseService.cpp
    src/DataWarehouseManager.h
    src/DataWarehouseManager.cpp
)

# BuildModule(
#     MODULE_NAME 
#         DataWarehouseService
#     TARGET_SOURCE_PRIVATE 
#         ${TARGET_SOURCE_FILES_PRIVATE}
#     TARGET_SOURCE_HEADER_BASE_DIR
#         ${CMAKE_CURRENT_SOURCE_DIR}/include
#     TARGET_SOURCE_PUBLIC_HEADER
#         ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
#     TARGET_DEPENDENICES_PRIVATE
#         MasterLog
#         CoreFrameworkInterface
#         DatabaseWrapper
#         ThreadPool
#     TARGET_DEPENDENICES_PUBLIC
#         ServiceExportMacro
#         ServiceDeclaration
#         NotificationHelper
#     TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
#         ${CMAKE_CURRENT_SOURCE_DIR}/include
#     TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
#         include
#     TARGET_INCLUDE_DIRECTORIES_PRIVATE
#         src
#     TARGET_DEFINITIONS
#         ${ADD_SERVICE_DEFINITIONS}
#     IDE_FOLDER
#         framework/Services
# )


set(TARGET_FACTORY_FILES
    src/DataWarehouseServiceFactory.cpp
)

set(TARGET_MODULE_NAME DataWarehouseService)

BuildUCFServiceModule(
    MODULE_NAME 
        ${TARGET_MODULE_NAME}
    INTERFACE_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    INTERFACE_FILES
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    INTERFACE_DEPENDENCIES
        ServiceExportMacro
        NotificationHelper
        ServiceDeclaration
    INTERFACE_INSTALL_DIR
        include
    IDE_FOLDER
        framework/Services
    IMPL_FILES
        ${TARGET_SOURCE_FILES_PRIVATE}
    IMPL_DEPENDENCIES_PUBLIC
        CoreFramework
    IMPL_DEPENDENCIES_PRIVATE
        NetworkService
        DatabaseWrapper
        MasterLog
        ThreadPool
    IMPL_BUILD_INCLUDE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    IMPL_BUILD_INCLUDE_DIR_PRIVATE
        src
    DLL_DEFINITIONS
        ${ADD_SERVICE_DEFINITIONS}
    FACTORY_FILES
        ${TARGET_FACTORY_FILES}
)