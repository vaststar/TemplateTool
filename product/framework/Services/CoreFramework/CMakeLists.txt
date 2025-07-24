include(BuildUCFServiceModule)


set(TARGET_MODULE_NAME CoreFramework)
set(TARGET_IDE_FOLDER framework/Services)

set(TARGET_INTERFACE_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(TARGET_INTERFACE_FILES #for include and install header
    include/ucf/CoreFramework/ICoreFramework.h
    include/ucf/CoreFramework/ICoreFramework.h
    include/ucf/CoreFramework/IServiceAccessor.h
    include/ucf/CoreFramework/ICoreFrameworkCallback.h
    include/ucf/CoreFramework/CoreFrameworkCallbackDefault.h
)
set(TARGET_INTERFACE_DEPENDENCIES 
    ServiceExportMacro
    NotificationHelper
    ServiceDeclaration
)
set(TARGET_INTERFACE_INSTALL_DIR include)


set(TARGET_IMPL_FILES
    src/CoreframeworkLogger.h
    src/ServiceAccessor.h
    src/CoreFramework.h
    src/CoreFramework.cpp
)
set(TARGET_IMPL_DEPENDENCIES_PRIVATE
    MasterLog
    ThreadPool
)
set(TARGET_IMPL_BUILD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TARGET_IMPL_BUILD_INCLUDE_DIR_PRIVATE src)
set(TARGET_FACTORY_FILES
    src/CoreFrameworkFactory.cpp
)

BuildUCFServiceModule(
    MODULE_NAME ${TARGET_MODULE_NAME}
    IDE_FOLDER ${TARGET_IDE_FOLDER}
    INTERFACE_BASE_DIR ${TARGET_INTERFACE_BASE_DIR}
    INTERFACE_FILES ${TARGET_INTERFACE_FILES}
    INTERFACE_DEPENDENCIES ${TARGET_INTERFACE_DEPENDENCIES}
    INTERFACE_INSTALL_DIR ${TARGET_INTERFACE_INSTALL_DIR}
    IMPL_FILES ${TARGET_IMPL_FILES}
    IMPL_DEPENDENCIES_PRIVATE ${TARGET_IMPL_DEPENDENCIES_PRIVATE}
    IMPL_BUILD_INCLUDE_DIR ${TARGET_IMPL_BUILD_INCLUDE_DIR}
    IMPL_BUILD_INCLUDE_DIR_PRIVATE ${TARGET_IMPL_BUILD_INCLUDE_DIR_PRIVATE}
    DLL_DEFINITIONS ${ADD_SERVICE_DEFINITIONS}
    FACTORY_FILES ${TARGET_FACTORY_FILES}
)
add_subdirectory(test)