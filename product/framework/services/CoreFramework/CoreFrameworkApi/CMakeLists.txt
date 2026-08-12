include(BuildInterface)

# CoreFrameworkApi (INTERFACE) - pure framework interfaces + the default callback.
set(MODULE_MODULE_NAME CoreFrameworkApi)
set(MODULE_IDE_FOLDER framework/services)
set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/CoreFramework/ICoreFramework.h
    include/ucf/CoreFramework/IServiceAccessor.h
    include/ucf/CoreFramework/ICoreFrameworkCallback.h
    include/ucf/CoreFramework/CoreFrameworkCallbackDefault.h
)
set(MODULE_DEPENDENCIES_PUBLIC
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
