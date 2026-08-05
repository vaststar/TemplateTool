include(BuildModule)
include(BuildInterface)

set(MODULE_IDE_FOLDER framework/Services)
set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ============================================================================
# DataWarehouseTypes (SHARED) - public value types / DTOs.
# These carry out-of-line implementations (ctors, operators) that consumers use
# by value, so they must live in a compiled library, not the pure-interface Api.
# ============================================================================
set(TYPES_PUBLIC_HEADERS
    include/ucf/Services/DataWarehouseService/DatabaseDataValue.h
    include/ucf/Services/DataWarehouseService/DatabaseDataRecord.h
    include/ucf/Services/DataWarehouseService/DatabaseTableModel.h
    include/ucf/Services/DataWarehouseService/DatabaseConfig.h
)
set(TYPES_SOURCE_PRIVATE
    src/DatabaseDataValue.cpp
    src/DatabaseDataRecord.cpp
    src/DatabaseTableModel.cpp
    src/DatabaseConfig.cpp
)
BuildModule(
    MODULE_NAME DataWarehouseTypes
    TARGET_SOURCE_PRIVATE ${TYPES_SOURCE_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
    TARGET_SOURCE_PUBLIC_HEADER ${TYPES_PUBLIC_HEADERS}
    TARGET_ADD_LINK_LIBRARY_PUBLIC ServiceExportMacro
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${MODULE_HEADER_BASE_DIR}
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE include
    TARGET_DEFINITIONS ${ADD_SERVICE_DEFINITIONS}
    IDE_FOLDER ${MODULE_IDE_FOLDER}
)

# ============================================================================
# DataWarehouseServiceApi (INTERFACE) - pure abstract interface, no binary.
# No SERVICE_EXPORT, no createInstance; consumers depend only on this.
# Publicly carries DataWarehouseTypes so callers get the DTO symbols.
# ============================================================================
set(API_PUBLIC_HEADERS
    include/ucf/Services/DataWarehouseService/IDataWarehouseService.h
    include/ucf/Services/DataWarehouseService/IDataWarehouseServiceCallback.h
    include/ucf/Services/DataWarehouseService/DatabaseModelDeclare.h
)
BuildInterface(
    MODULE_NAME DataWarehouseServiceApi
    TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
    TARGET_SOURCE_PUBLIC_HEADER ${API_PUBLIC_HEADERS}
    TARGET_ADD_LINK_LIBRARY_PUBLIC
        DataWarehouseTypes
        ServiceDeclaration
        NotificationHelper
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${MODULE_HEADER_BASE_DIR}
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE include
    IDE_FOLDER ${MODULE_IDE_FOLDER}
)

# ============================================================================
# DataWarehouseServiceImpl (SHARED) - the concrete service implementation.
# Only ServiceFactory links this (to create instances via createDataWarehouseService).
# ============================================================================
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
    IDE_FOLDER ${MODULE_IDE_FOLDER}
)

add_subdirectory(test)
