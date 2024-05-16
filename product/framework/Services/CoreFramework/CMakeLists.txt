include(BuildModule)

set(target_public_include_headers  #for include and install header
    include/CoreFramework/IService.h
    include/CoreFramework/ICoreFramework.h
    include/CoreFramework/ICoreFramework.h
    include/CoreFramework/ServiceAccessor.h
    include/CoreFramework/ICoreFrameworkCallback.h
)

set(target_build_source_files
    src/CoreFramework.h
    src/CoreFramework.cpp
)

BuildModule(
    MODULE_NAME 
        CoreFramework
    TARGET_SOURCE_PRIVATE 
        ${target_build_source_files}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_LIST_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${target_public_include_headers}
    TARGET_DEPENDENICES_PRIVATE
        ThreadPool
    TARGET_DEPENDENICES_PUBLIC
        ServiceCommonFile
        NotificationHelper
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    TARGET_DEFINITIONS
        ${ADD_SERVICE_DEFINITIONS}
    # INSTALL_PUBLIC_HEADER 
    #     ${target_public_include_headers}
    # INSTALL_PUBLIC_HEADER_DIR
    #     include/CoreFramework
    IDE_FOLDER
        framework/Services
)