include(BuildInterface)

# DataWarehouseServiceApi (INTERFACE) - pure abstract interface, no binary.
# No SERVICE_EXPORT, no createInstance; consumers depend only on this.
# Publicly carries DataWarehouseTypes so callers get the DTO symbols.
set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(API_PUBLIC_HEADERS
    include/ucf/Services/DataWarehouseService/IDataWarehouseService.h
    include/ucf/Services/DataWarehouseService/IDataWarehouseServiceCallback.h
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
    IDE_FOLDER framework/Services
)
