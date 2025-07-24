include(BuildUCFServiceModule)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}") 

add_custom_target(update_version)
set_target_properties(update_version PROPERTIES FOLDER framework/Services)

add_custom_command( TARGET update_version PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/appVersion.h.in ${CMAKE_BINARY_DIR}/appVersion.h.in
    COMMAND ${CMAKE_COMMAND} -P  ${CMAKE_CURRENT_SOURCE_DIR}/UpdateVersion.cmake
    ALWAYS
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
) 

set(TARGET_MODULE_NAME ClientInfoService)
set(TARGET_IDE_FOLDER framework/Services)

set(TARGET_INTERFACE_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(TARGET_INTERFACE_FILES #for include and install header
    include/ucf/Services/ClientInfoService/IClientInfoServiceCallback.h
    include/ucf/Services/ClientInfoService/IClientInfoService.h
    include/ucf/Services/ClientInfoService/ClientInfoModel.h
)
set(TARGET_INTERFACE_DEPENDENCIES 
    ServiceExportMacro
    NotificationHelper
    ServiceDeclaration
)
set(TARGET_INTERFACE_INSTALL_DIR include)


set(TARGET_IMPL_FILES
    src/ClientInfoServiceLogger.h
    src/ClientInfoService.h
    src/ClientInfoService.cpp
    src/ClientInfoManager.h
    src/ClientInfoManager.cpp
)
set(TARGET_IMPL_DEPENDENCIES_PUBLIC
    DataWarehouseService
    CoreFramework
)
set(TARGET_IMPL_DEPENDENCIES_PRIVATE
    DataWarehouseSchema
    MasterLog
    TimeUtils
    OSUtils
    StringUtils
    UUIDUtils
    magic_enum
)
set(TARGET_IMPL_BUILD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TARGET_IMPL_BUILD_INCLUDE_DIR_PRIVATE src ${CMAKE_BINARY_DIR})
set(TARGET_FACTORY_FILES
    src/ClientInfoServiceFactory.cpp
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

add_dependencies(ClientInfoServiceImpl update_version)