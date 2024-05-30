include(BuildModule)

set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC  #for include and install header
    include/ucf/CoreFramework/IService.h
    include/ucf/CoreFramework/ICoreFramework.h
    include/ucf/CoreFramework/ICoreFramework.h
    include/ucf/CoreFramework/IServiceAccessor.h
    include/ucf/CoreFramework/ICoreFrameworkCallback.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/ServiceAccessor.h
    src/CoreFramework.h
    src/CoreFramework.cpp
)

BuildModule(
    MODULE_NAME 
        CoreFramework
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PRIVATE
        ThreadPool
    TARGET_DEPENDENICES_PUBLIC
        ServiceCommonFile
        NotificationHelper
        libcurl
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