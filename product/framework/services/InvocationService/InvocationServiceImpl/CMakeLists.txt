include(BuildModule)

# InvocationServiceImpl (SHARED) - the concrete service implementation.
# Only ServiceFactory links this (to create instances via createInvocationService).
set(MODULE_MODULE_NAME InvocationServiceImpl)
set(MODULE_IDE_FOLDER framework/Services)
set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/Services/InvocationService/InvocationServiceCreator.h
)
set(MODULE_SOURCE_PRIVATE
    src/InvocationServiceLogger.h
    src/InvocationService.h
    src/InvocationService.cpp
    src/InvocationManager.h
    src/InvocationManager.cpp
)
set(MODULE_DEPENDENCIES_PUBLIC
    InvocationServiceApi
    ServiceExportMacro
)
set(MODULE_DEPENDENCIES_PRIVATE
    CoreFramework
    TimeUtils
    OSUtils
    StringUtils
    UUIDUtils
    LoggerUtils
)
set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR src)

BuildModule(
    MODULE_NAME ${MODULE_MODULE_NAME}
    TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
    TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
    TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
    TARGET_ADD_LINK_LIBRARY_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${MODULE_BUILD_INTERFACE_DIR}
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
    TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
    TARGET_DEFINITIONS ${ADD_SERVICE_DEFINITIONS}
    IDE_FOLDER ${MODULE_IDE_FOLDER}
)

add_subdirectory(test)
