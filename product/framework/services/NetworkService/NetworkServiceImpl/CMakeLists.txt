include(BuildModule)

# NetworkServiceImpl - concrete NetworkService implementation.
set(MODULE_MODULE_NAME NetworkServiceImpl)
set(MODULE_IDE_FOLDER framework/services)
set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/services/NetworkService/NetworkServiceImplExport.h
    include/ucf/services/NetworkService/NetworkServiceCreator.h
)
set(MODULE_SOURCE_PRIVATE
    src/NetworkServiceLogger.h
    src/NetworkService.h
    src/NetworkService.cpp
    src/NetworkHttpManager.h
    src/NetworkHttpManager.cpp
    src/NetworkHttpHandler/INetworkHttpHandler.h
    src/NetworkHttpHandler/NetworkHttpTypeConverter.h
    src/NetworkHttpHandler/NetworkHttpRestHandler.h
    src/NetworkHttpHandler/NetworkHttpRestHandler.cpp
    src/NetworkHttpHandler/NetworkHttpRawHandler.h
    src/NetworkHttpHandler/NetworkHttpRawHandler.cpp
    src/NetworkHttpHandler/NetworkHttpDownloadToMemoryHandler.h
    src/NetworkHttpHandler/NetworkHttpDownloadToMemoryHandler.cpp
    src/NetworkHttpHandler/NetworkHttpDownloadToFileHandler.h
    src/NetworkHttpHandler/NetworkHttpDownloadToFileHandler.cpp
)
set(MODULE_DEPENDENCIES_PUBLIC
    NetworkServiceApi
)
set(MODULE_DEPENDENCIES_PRIVATE
    CoreFrameworkApi
    UUIDUtils
    OSUtils
    LibCurlClient
    LoggerUtils
)

set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR src)

set(MODULE_PUBLIC_DEFINITIONS)
set(MODULE_PRIVATE_DEFINITIONS)
if (SERVICE_LIB_TYPE STREQUAL "STATIC_LIB")
    set(MODULE_PUBLIC_DEFINITIONS NETWORK_SERVICE_IMPL_STATIC)
else()
    set(MODULE_PRIVATE_DEFINITIONS NETWORK_SERVICE_IMPL_SHARED)
endif()

BuildModule(
    ${SERVICE_LIB_TYPE}
    MODULE_NAME ${MODULE_MODULE_NAME}
    TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
    TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
    TARGET_ADD_LINK_LIBRARY_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${MODULE_BUILD_INTERFACE_DIR}
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
    TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
    TARGET_PUBLIC_DEFINITIONS ${MODULE_PUBLIC_DEFINITIONS}
    TARGET_PRIVATE_DEFINITIONS ${MODULE_PRIVATE_DEFINITIONS}
    IDE_FOLDER ${MODULE_IDE_FOLDER}
)
