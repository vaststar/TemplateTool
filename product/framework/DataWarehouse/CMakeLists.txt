include(BuildModule)
if (WIN32)
    set(ADD_DATAWAREHOUSE_DEFINITIONS DATAWAREHOUSE_DLL DATAWAREHOUSE_LIB)
else()
    set(ADD_DATAWAREHOUSE_DEFINITIONS )
endif()

set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC  #for include and install header
    include/ucf/DataWarehouse/IDataWarehouse.h
    include/ucf/DataWarehouse/DataWarehouseExport.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/DataWarehouse.h
    src/DataWarehouse.cpp
    src/DataWarehouseLogger.h
    src/DataWarehouseSchemas.h
    src/DataWarehouseSchemas.cpp
)

BuildModule(
    MODULE_NAME 
        DataWarehouse
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PRIVATE
        DatabaseWrapper
        ThreadPool
    TARGET_DEPENDENICES_PUBLIC
        MasterLog
        NotificationHelper
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    TARGET_DEFINITIONS
        ${ADD_DATAWAREHOUSE_DEFINITIONS}
    IDE_FOLDER
        framework/DataWarehouse
)