include(BuildModule)

set(TARGET_MODULE_NAME ContactService)
set(TARGET_IDE_FOLDER framework/Services)

set(TARGET_INTERFACE_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(TARGET_INTERFACE_FILES #for include and install header
    include/ucf/Services/ContactService/IContactServiceCallback.h
    include/ucf/Services/ContactService/IContactService.h
    include/ucf/Services/ContactService/IContactEntities.h
)
set(TARGET_INTERFACE_DEPENDENCIES 
    ServiceExportMacro
    NotificationHelper
    ServiceDeclaration
)
set(TARGET_INTERFACE_INSTALL_DIR include)


set(TARGET_IMPL_FILES
    src/ContactServiceLogger.h
    src/ContactService.h
    src/ContactService.cpp
    src/ContactModel.h
    src/ContactModel.cpp
    src/ContactEntities.h
    src/ContactEntities.cpp
    src/ContactManager.h
    src/ContactManager.cpp
    src/Adapters/ContactAdapter.h
    src/Adapters/ContactAdapter.cpp
)
set(TARGET_IMPL_DEPENDENCIES_PUBLIC
    CoreFramework
)
set(TARGET_IMPL_DEPENDENCIES_PRIVATE
    NetworkService
    DatabaseWrapper
    MasterLog
    ThreadPool
    nlohmann_json::nlohmann_json
)
set(TARGET_IMPL_BUILD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TARGET_IMPL_BUILD_INCLUDE_DIR_PRIVATE src)
set(TARGET_FACTORY_FILES
    src/ContactServiceFactory.cpp
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
