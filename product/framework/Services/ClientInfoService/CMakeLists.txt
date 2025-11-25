add_subdirectory(version)
include(BuildModule)

set(MODULE_MODULE_NAME ClientInfoService)
set(MODULE_IDE_FOLDER framework/Services)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES #for include and install header
    include/ucf/Services/ClientInfoService/IClientInfoServiceCallback.h
    include/ucf/Services/ClientInfoService/IClientInfoService.h
    include/ucf/Services/ClientInfoService/ClientInfoModel.h
)
set(MODULE_SOURCE_PRIVATE
    src/ClientInfoServiceLogger.h
    src/ClientInfoService.h
    src/ClientInfoService.cpp
    src/ClientInfoManager.h
    src/ClientInfoManager.cpp
)
set(MODULE_DEPENDENCIES_PUBLIC
    ServiceExportMacro
    NotificationHelper
    ServiceDeclaration
)
set(MODULE_DEPENDENCIES_PRIVATE
    DataWarehouseService
    CoreFramework
    DataWarehouseSchema
    AppVersionHeader
    MasterLog
    TimeUtils
    OSUtils
    StringUtils
    UUIDUtils
    magic_enum
)

set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR src)

BuildModule(
MODULE_NAME ${MODULE_MODULE_NAME}
TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
TARGET_ADD_LINK_LIBRARY_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE  ${MODULE_BUILD_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
TARGET_DEFINITIONS ${ADD_SERVICE_DEFINITIONS}
IDE_FOLDER ${MODULE_IDE_FOLDER}
)

add_subdirectory(test)
