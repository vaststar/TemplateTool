include(BuildModule)
if (WIN32)
    set(ADD_LIBCURL_DEFINITIONS LIBCURLCLIENT_DLL) #LIBCURLCLIENT_DLL LIBCURLCLIENT_LIB
else()
    set(ADD_LIBCURL_DEFINITIONS)
endif()

set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC  #for include and install header
    include/ucf/Utilities/NetworkUtils/LibCurlClient/LibCurlClient.h
    include/ucf/Utilities/NetworkUtils/LibCurlClient/LibCurlClientExport.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/LibCurlClient.cpp
    src/LibCurlClientLogger.h
    src/LibCurlMultiHandleManager.h
    src/LibCurlMultiHandleManager.cpp
    src/LibCurlMultiHandle.h
    src/LibCurlMultiHandle.cpp
    src/LibCurlEasyHandle.h
    src/LibCurlEasyHandle.cpp
    src/LibCurlPayloadData.h
    src/LibCurlPayloadData.cpp
)

BuildModule(
    MODULE_NAME 
        LibCurlClient
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENCIES_PUBLIC
        NetworkModelTypes
        StringUtils
    TARGET_DEPENDENCIES_PRIVATE
        MasterLog
        openssl
        curl
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    TARGET_DEFINITIONS
        ${ADD_LIBCURL_DEFINITIONS}
    IDE_FOLDER
        framework/Utilities/NetworkUtils
)