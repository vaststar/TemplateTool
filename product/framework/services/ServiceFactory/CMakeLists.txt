include(BuildModule)

set(MODULE_MODULE_NAME ServiceFactory)
set(MODULE_IDE_FOLDER framework/Services)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES #for include and install header
    include/ucf/Services/ServiceFactory/ServiceFactoryExport.h
    include/ucf/Services/ServiceFactory/IServiceFactory.h
)
set(MODULE_SOURCE_PRIVATE
    src/ServiceFactoryLogger.h
    src/ServiceFactory.h
    src/ServiceFactory.cpp
)
set(MODULE_DEPENDENCIES_PUBLIC
    ServiceDeclaration
)

set(MODULE_DEPENDENCIES_PRIVATE
    MiniAppServiceImpl
    UpgradeServiceImpl
    CameraDirectoryServiceImpl
    FeatureSettingsServiceImpl
    PerformanceServiceImpl
    StabilityServiceImpl
    MediaService
    ImageServiceImpl
    ContactServiceImpl
    NetworkServiceImpl
    ClientInfoServiceImpl
    DataWarehouseServiceImpl
    InvocationServiceImpl
    CoreFrameworkApi
    LoggerUtils
)

set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR src)

set(MODULE_PUBLIC_DEFINITIONS)
set(MODULE_PRIVATE_DEFINITIONS)
if (SERVICE_LIB_TYPE STREQUAL "STATIC_LIB")
    # Static lib: macro is empty but must propagate to consumers -> PUBLIC
    set(MODULE_PUBLIC_DEFINITIONS SERVICE_FACTORY_STATIC)
else()
    # Shared lib: only this module exports; consumers use dllimport -> PRIVATE
    set(MODULE_PRIVATE_DEFINITIONS SERVICE_FACTORY_SHARED)
endif()

BuildModule(
${SERVICE_LIB_TYPE}
MODULE_NAME ${MODULE_MODULE_NAME}
TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
TARGET_ADD_LINK_LIBRARY_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE  ${MODULE_BUILD_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
TARGET_PUBLIC_DEFINITIONS ${MODULE_PUBLIC_DEFINITIONS}
TARGET_PRIVATE_DEFINITIONS ${MODULE_PRIVATE_DEFINITIONS}
IDE_FOLDER ${MODULE_IDE_FOLDER}
)

add_subdirectory(test)
