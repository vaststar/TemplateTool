include(BuildModule)

# DataWarehouseTypes (SHARED) - public value types / DTOs.
# These carry out-of-line implementations (ctors, operators) that consumers use
# by value, so they live in a compiled library rather than the pure-interface Api.
set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(TYPES_PUBLIC_HEADERS
    include/ucf/Services/DataWarehouseService/DatabaseDataValue.h
    include/ucf/Services/DataWarehouseService/DatabaseDataRecord.h
    include/ucf/Services/DataWarehouseService/DatabaseTableModel.h
    include/ucf/Services/DataWarehouseService/DatabaseConfig.h
    include/ucf/Services/DataWarehouseService/DatabaseModelDeclare.h
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
    IDE_FOLDER framework/Services
)
