add_subdirectory(LibCurlClient)
include(BuildModule)

set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC  #for include and install header
    include/ucf/Services/NetworkService/INetworkService.h
    include/ucf/Services/NetworkService/INetworkServiceCallback.h
    include/ucf/Services/NetworkService/Http/INetworkHttpManager.h
    include/ucf/Services/NetworkService/Http/NetworkHttpTypes.h
    include/ucf/Services/NetworkService/Http/NetworkHttpRequest.h
    include/ucf/Services/NetworkService/Http/NetworkHttpResponse.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/NetworkService.h
    src/NetworkService.cpp
    src/NetworkHttpRequest.cpp
    src/NetworkHttpResponse.cpp
    src/NetworkHttpManager.h
    src/NetworkHttpManager.cpp
)

BuildModule(
    MODULE_NAME 
        NetworkService
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PRIVATE
        TimeUtils
        CoreFramework
        LibCurlClient
    TARGET_DEPENDENICES_PUBLIC
        ServiceCommonFile
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    TARGET_DEFINITIONS
        ${ADD_SERVICE_DEFINITIONS}
    IDE_FOLDER
        framework/Services
)