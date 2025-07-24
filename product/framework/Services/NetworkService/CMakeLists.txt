include(BuildUCFServiceModule)

set(TARGET_MODULE_NAME NetworkService)
set(TARGET_IDE_FOLDER framework/Services)

set(TARGET_INTERFACE_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(TARGET_INTERFACE_FILES #for include and install header
    include/ucf/Services/NetworkService/INetworkService.h
    include/ucf/Services/NetworkService/INetworkServiceCallback.h
    include/ucf/Services/NetworkService/Http/INetworkHttpManager.h
    include/ucf/Services/NetworkService/Model/HttpRestRequest.h
    include/ucf/Services/NetworkService/Model/HttpRestResponse.h
    include/ucf/Services/NetworkService/Model/HttpDeclareTypes.h
    include/ucf/Services/NetworkService/Model/HttpRawRequest.h
    include/ucf/Services/NetworkService/Model/HttpRawResponse.h
    include/ucf/Services/NetworkService/Model/HttpDownloadToMemoryRequest.h
    include/ucf/Services/NetworkService/Model/HttpDownloadToMemoryResponse.h
    include/ucf/Services/NetworkService/Model/HttpDownloadToFileRequest.h
    include/ucf/Services/NetworkService/Model/HttpDownloadToFileResponse.h
)
set(TARGET_INTERFACE_DEPENDENCIES 
    ServiceExportMacro
    NotificationHelper
    ServiceDeclaration
)
set(TARGET_INTERFACE_INSTALL_DIR include)

set(TARGET_IMPL_FILES
    src/NetworkServiceLogger.h
    src/NetworkService.h
    src/NetworkService.cpp
    src/NetworkHttpManager.h
    src/NetworkHttpManager.cpp
    src/NetworkHttpHandler/INetworkHttpHandler.h
    src/NetworkHttpHandler/NetworkHttpRestHandler.h
    src/NetworkHttpHandler/NetworkHttpRestHandler.cpp
    src/NetworkHttpHandler/NetworkHttpRawHandler.h
    src/NetworkHttpHandler/NetworkHttpRawHandler.cpp
    src/NetworkHttpHandler/NetworkHttpDownloadToMemoryHandler.h
    src/NetworkHttpHandler/NetworkHttpDownloadToMemoryHandler.cpp
    src/NetworkHttpHandler/NetworkHttpDownloadToFileHandler.h
    src/NetworkHttpHandler/NetworkHttpDownloadToFileHandler.cpp
    src/HttpRestRequest.cpp
    src/HttpRestResponse.cpp
    src/HttpRawRequest.cpp
    src/HttpRawResponse.cpp
    src/HttpDownloadToMemoryRequest.cpp
    src/HttpDownloadToMemoryResponse.cpp
    src/HttpDownloadToFileRequest.cpp
    src/HttpDownloadToFileResponse.cpp
)
set(TARGET_IMPL_DEPENDENCIES_PUBLIC
    CoreFramework
)
set(TARGET_IMPL_DEPENDENCIES_PRIVATE
    MasterLog
    LibCurlClient
    UUIDUtils
)
set(TARGET_IMPL_BUILD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TARGET_IMPL_BUILD_INCLUDE_DIR_PRIVATE src)
set(TARGET_FACTORY_FILES
    src/NetworkServiceFactory.cpp
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
