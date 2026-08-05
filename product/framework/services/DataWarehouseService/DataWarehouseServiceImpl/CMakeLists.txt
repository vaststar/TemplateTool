include(BuildModule)

# DataWarehouseServiceImpl (SHARED) - the concrete service implementation.
# Only ServiceFactory links this (to create instances via createDataWarehouseService).
set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(IMPL_PUBLIC_HEADERS
    include/ucf/Services/DataWarehouseService/DataWarehouseServiceCreator.h
)
set(IMPL_SOURCE_PRIVATE
    src/DataWarehouseServiceLogger.h
    src/DataWarehouseService.h
    src/DataWarehouseService.cpp
    src/DataWarehouseManager.h
    src/DataWarehouseManager.cpp
)

BuildModule(
    MODULE_NAME DataWarehouseServiceImpl
    TARGET_SOURCE_PRIVATE ${IMPL_SOURCE_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
    TARGET_SOURCE_PUBLIC_HEADER ${IMPL_PUBLIC_HEADERS}
    TARGET_ADD_LINK_LIBRARY_PUBLIC
        DataWarehouseServiceApi
        ServiceExportMacro
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        NetworkService
        DatabaseClient
        CoreFramework
        LoggerUtils
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${MODULE_HEADER_BASE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE src
    TARGET_DEFINITIONS ${ADD_SERVICE_DEFINITIONS}
    IDE_FOLDER framework/Services
)

add_subdirectory(test)
