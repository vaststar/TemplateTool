include(BuildModule)

# ContactServiceImpl (SHARED) - the concrete service implementation.
# Only ServiceFactory links this (to create instances via createContactService).
set(MODULE_MODULE_NAME ContactServiceImpl)
set(MODULE_IDE_FOLDER framework/Services)
set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/Services/ContactService/ContactServiceImplExport.h
    include/ucf/Services/ContactService/ContactServiceCreator.h
)
set(MODULE_SOURCE_PRIVATE
    src/ContactServiceLogger.h
    src/ContactService.h
    src/ContactService.cpp
    src/ContactNotificationSink.h
    src/ContactModel.h
    src/ContactModel.cpp
    src/ContactEntities.h
    src/ContactEntities.cpp
    src/ContactManager.h
    src/ContactManager.cpp
    src/ContactDBAccess.h
    src/ContactDBAccess.cpp
    src/Adapters/ContactAdapter.h
    src/Adapters/ContactAdapter.cpp
)
set(MODULE_DEPENDENCIES_PUBLIC
    ContactServiceApi
)
set(MODULE_DEPENDENCIES_PRIVATE
    NetworkServiceApi
    DatabaseClient
    DataWarehouseServiceApi
    DataWarehouseSchema
    ClientInfoServiceApi
    CoreFramework
    LoggerUtils
    ThreadPool
    UUIDUtils
    SinkNotifier
)
set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR src)

set(MODULE_PUBLIC_DEFINITIONS)
set(MODULE_PRIVATE_DEFINITIONS)
if (SERVICE_LIB_TYPE STREQUAL "STATIC_LIB")
    set(MODULE_PUBLIC_DEFINITIONS CONTACT_SERVICE_IMPL_STATIC)
else()
    set(MODULE_PRIVATE_DEFINITIONS CONTACT_SERVICE_IMPL_SHARED)
endif()

BuildModule(
    ${SERVICE_LIB_TYPE}
    MODULE_NAME ${MODULE_MODULE_NAME}
    TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
    TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
    TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
    TARGET_ADD_LINK_LIBRARY_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${MODULE_BUILD_INTERFACE_DIR}
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
    TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
    TARGET_PUBLIC_DEFINITIONS ${MODULE_PUBLIC_DEFINITIONS}
    TARGET_PRIVATE_DEFINITIONS ${MODULE_PRIVATE_DEFINITIONS}
    IDE_FOLDER ${MODULE_IDE_FOLDER}
)

