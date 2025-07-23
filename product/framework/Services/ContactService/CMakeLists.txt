include(BuildModule)

set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC  #for include and install header
    include/ucf/Services/ContactService/IContactServiceCallback.h
    include/ucf/Services/ContactService/IContactService.h
    include/ucf/Services/ContactService/IContactEntities.h
)

set(TARGET_SOURCE_FILES_PRIVATE
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


set(TARGET_FACTORY_FILES
    src/ContactServiceFactory.cpp
)

set(TARGET_MODULE_NAME ContactService)

BuildUCFServiceModule(
    MODULE_NAME 
        ${TARGET_MODULE_NAME}
    INTERFACE_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    INTERFACE_FILES
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    INTERFACE_DEPENDENCIES
        ServiceExportMacro
        NotificationHelper
        ServiceDeclaration
    INTERFACE_INSTALL_DIR
        include
    IDE_FOLDER
        framework/Services
    IMPL_FILES
        ${TARGET_SOURCE_FILES_PRIVATE}
    IMPL_DEPENDENCIES_PUBLIC
        CoreFramework
    IMPL_DEPENDENCIES_PRIVATE
        NetworkService
        DatabaseWrapper
        MasterLog
        ThreadPool
        nlohmann_json::nlohmann_json
    IMPL_BUILD_INCLUDE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    IMPL_BUILD_INCLUDE_DIR_PRIVATE
        src
    DLL_DEFINITIONS
        ${ADD_SERVICE_DEFINITIONS}
    FACTORY_FILES
        ${TARGET_FACTORY_FILES}
)

# BuildModule(
#     MODULE_NAME 
#         ContactService
#     TARGET_SOURCE_PRIVATE 
#         ${TARGET_SOURCE_FILES_PRIVATE}
#     TARGET_SOURCE_HEADER_BASE_DIR
#         ${CMAKE_CURRENT_SOURCE_DIR}/include
#     TARGET_SOURCE_PUBLIC_HEADER
#         ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
#     TARGET_DEPENDENICES_PRIVATE
#         nlohmann_json::nlohmann_json
#         MasterLog
#         CoreFrameworkInterface
#         NetworkService
#         DatabaseWrapper
#     TARGET_DEPENDENICES_PUBLIC
#         ServiceExportMacro
#         ServiceDeclaration
#         NotificationHelper
#     TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
#         ${CMAKE_CURRENT_SOURCE_DIR}/include
#     TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
#         include
#     TARGET_INCLUDE_DIRECTORIES_PRIVATE
#         src
#     TARGET_DEFINITIONS
#         ${ADD_SERVICE_DEFINITIONS}
#     IDE_FOLDER
#         framework/Services
# )