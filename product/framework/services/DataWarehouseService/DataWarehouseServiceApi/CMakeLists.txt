include(BuildInterface)

# DataWarehouseServiceApi (INTERFACE) - pure abstract interface, no binary.
# No SERVICE_EXPORT, no createInstance; consumers depend only on this.
# Publicly carries DataWarehouseTypes so callers get the DTO symbols.
set(MODULE_MODULE_NAME DataWarehouseServiceApi)
set(MODULE_IDE_FOLDER framework/Services)
set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/Services/DataWarehouseService/IDataWarehouseService.h
    include/ucf/Services/DataWarehouseService/IDataWarehouseServiceCallback.h
)
set(MODULE_DEPENDENCIES_PUBLIC
    DataWarehouseTypes
    ServiceDeclaration
    NotificationHelper
)
set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)

BuildInterface(
    MODULE_NAME ${MODULE_MODULE_NAME}
    TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
    TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
    TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${MODULE_BUILD_INTERFACE_DIR}
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
    IDE_FOLDER ${MODULE_IDE_FOLDER}
)
