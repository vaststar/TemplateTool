include(BuildModule)

# DataWarehouseTypes (SHARED) - public value types / DTOs.
# These carry out-of-line implementations (ctors, operators) that consumers use
# by value, so they live in a compiled library rather than the pure-interface Api.
set(MODULE_MODULE_NAME DataWarehouseTypes)
set(MODULE_IDE_FOLDER framework/Services)
set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/Services/DataWarehouseService/DatabaseDataValue.h
    include/ucf/Services/DataWarehouseService/DatabaseDataRecord.h
    include/ucf/Services/DataWarehouseService/DatabaseTableModel.h
    include/ucf/Services/DataWarehouseService/DatabaseConfig.h
    include/ucf/Services/DataWarehouseService/DatabaseModelDeclare.h
)
set(MODULE_SOURCE_PRIVATE
    src/DatabaseDataValue.cpp
    src/DatabaseDataRecord.cpp
    src/DatabaseTableModel.cpp
    src/DatabaseConfig.cpp
)
set(MODULE_DEPENDENCIES_PUBLIC
    ServiceExportMacro
)
set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)

BuildModule(
    MODULE_NAME ${MODULE_MODULE_NAME}
    TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
    TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
    TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${MODULE_BUILD_INTERFACE_DIR}
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
    TARGET_DEFINITIONS ${ADD_SERVICE_DEFINITIONS}
    IDE_FOLDER ${MODULE_IDE_FOLDER}
)
