include(BuildModule)

set(TARGET_MODULE_NAME DataWarehouseService)
set(TARGET_IDE_FOLDER framework/Services)

set(TARGET_INTERFACE_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(TARGET_INTERFACE_FILES #for include and install header
    include/ucf/Services/DataWarehouseService/DatabaseModelDeclare.h
    include/ucf/Services/DataWarehouseService/IDataWarehouseService.h
    include/ucf/Services/DataWarehouseService/DataBaseConfig.h
    include/ucf/Services/DataWarehouseService/DataBaseTableModel.h
    include/ucf/Services/DataWarehouseService/DataBaseDataValue.h
    include/ucf/Services/DataWarehouseService/DatabaseDataRecord.h
    include/ucf/Services/DataWarehouseService/IDataWarehouseServiceCallback.h
)
set(TARGET_INTERFACE_DEPENDENCIES 
    ServiceExportMacro
    NotificationHelper
    ServiceDeclaration
)
set(TARGET_INTERFACE_INSTALL_DIR include)


set(TARGET_IMPL_FILES
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
set(TARGET_IMPL_DEPENDENCIES_PUBLIC
    CoreFramework
)
set(TARGET_IMPL_DEPENDENCIES_PRIVATE
    NetworkService
    DatabaseWrapper
    MasterLog
    ThreadPool
)
set(TARGET_IMPL_BUILD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TARGET_IMPL_BUILD_INCLUDE_DIR_PRIVATE src)
set(TARGET_FACTORY_FILES
    src/DataWarehouseServiceFactory.cpp
)

BuildUCFServiceModule(
    MODULE_NAME ${TARGET_MODULE_NAME}
    IDE_FOLDER ${TARGET_IDE_FOLDER}
    INTERFACE_BASE_DIR ${TARGET_INTERFACE_BASE_DIR}
    INTERFACE_FILES ${TARGET_INTERFACE_FILES}
    INTERFACE_DEPENDENCIES ${TARGET_INTERFACE_DEPENDENCIES}
    INTERFACE_INSTALL_DIR ${TARGET_INTERFACE_INSTALL_DIR}
    IMPL_FILES ${TARGET_IMPL_FILES}
    IMPL_DEPENDENCIES_PUBLIC ${TARGET_IMPL_DEPENDENCIES_PUBLIC}
    IMPL_DEPENDENCIES_PRIVATE ${TARGET_IMPL_DEPENDENCIES_PRIVATE}
    IMPL_BUILD_INCLUDE_DIR ${TARGET_IMPL_BUILD_INCLUDE_DIR}
    IMPL_BUILD_INCLUDE_DIR_PRIVATE ${TARGET_IMPL_BUILD_INCLUDE_DIR_PRIVATE}
    DLL_DEFINITIONS ${ADD_SERVICE_DEFINITIONS}
    FACTORY_FILES ${TARGET_FACTORY_FILES}
)
