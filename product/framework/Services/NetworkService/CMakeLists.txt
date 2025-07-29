include(BuildModule)

set(MODULE_MODULE_NAME NetworkService)
set(MODULE_IDE_FOLDER framework/Services)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES #for include and install header
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
set(MODULE_SOURCE_PRIVATE
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
set(MODULE_DEPENDENCIES_PUBLIC
    ServiceExportMacro
    NotificationHelper
    ServiceDeclaration
)
set(MODULE_DEPENDENCIES_PRIVATE
    CoreFramework
    UUIDUtils
    OSUtils
    LibCurlClient
    MasterLog
    ThreadPool
)

set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR src)

BuildModule(
MODULE_NAME ${MODULE_MODULE_NAME}
TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
TARGET_DEPENDENCIES_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
TARGET_DEPENDENCIES_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE  ${MODULE_BUILD_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
TARGET_DEFINITIONS ${ADD_SERVICE_DEFINITIONS}
IDE_FOLDER ${MODULE_IDE_FOLDER}
)


