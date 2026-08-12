include(BuildModule)

# NetworkServiceTypes - public value types (pimpl request/response models).
set(MODULE_MODULE_NAME NetworkServiceTypes)
set(MODULE_IDE_FOLDER framework/services)
set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/services/NetworkService/NetworkServiceTypesExport.h
    include/ucf/services/NetworkService/model/HttpDeclareTypes.h
    include/ucf/services/NetworkService/model/HttpRestRequest.h
    include/ucf/services/NetworkService/model/HttpRestResponse.h
    include/ucf/services/NetworkService/model/HttpRawRequest.h
    include/ucf/services/NetworkService/model/HttpRawResponse.h
    include/ucf/services/NetworkService/model/HttpDownloadToMemoryRequest.h
    include/ucf/services/NetworkService/model/HttpDownloadToMemoryResponse.h
    include/ucf/services/NetworkService/model/HttpDownloadToFileRequest.h
    include/ucf/services/NetworkService/model/HttpDownloadToFileResponse.h
)
set(MODULE_SOURCE_PRIVATE
    src/HttpRestRequest.cpp
    src/HttpRestResponse.cpp
    src/HttpRawRequest.cpp
    src/HttpRawResponse.cpp
    src/HttpDownloadToMemoryRequest.cpp
    src/HttpDownloadToMemoryResponse.cpp
    src/HttpDownloadToFileRequest.cpp
    src/HttpDownloadToFileResponse.cpp
)
set(MODULE_DEPENDENCIES_PRIVATE
    UUIDUtils
)
set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR src)

set(MODULE_PUBLIC_DEFINITIONS)
set(MODULE_PRIVATE_DEFINITIONS)
if (SERVICE_LIB_TYPE STREQUAL "STATIC_LIB")
    set(MODULE_PUBLIC_DEFINITIONS NETWORK_SERVICE_TYPES_STATIC)
else()
    set(MODULE_PRIVATE_DEFINITIONS NETWORK_SERVICE_TYPES_SHARED)
endif()

BuildModule(
    ${SERVICE_LIB_TYPE}
    MODULE_NAME ${MODULE_MODULE_NAME}
    TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
    TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
    TARGET_ADD_LINK_LIBRARY_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${MODULE_BUILD_INTERFACE_DIR}
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
    TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
    TARGET_PUBLIC_DEFINITIONS ${MODULE_PUBLIC_DEFINITIONS}
    TARGET_PRIVATE_DEFINITIONS ${MODULE_PRIVATE_DEFINITIONS}
    IDE_FOLDER ${MODULE_IDE_FOLDER}
)
