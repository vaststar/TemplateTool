include(BuildModule)

set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC  #for include and install header
    include/ucf/CoreFramework/ICoreFramework.h
    include/ucf/CoreFramework/ICoreFramework.h
    include/ucf/CoreFramework/IServiceAccessor.h
    include/ucf/CoreFramework/ICoreFrameworkCallback.h
    include/ucf/CoreFramework/CoreFrameworkCallbackDefault.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/CoreframeworkLogger.h
    src/ServiceAccessor.h
    src/CoreFramework.h
    src/CoreFramework.cpp
)

# BuildModule(
#     MODULE_NAME 
#         CoreFramework
#     TARGET_SOURCE_PRIVATE 
#         ${TARGET_SOURCE_FILES_PRIVATE}
#     TARGET_SOURCE_HEADER_BASE_DIR
#         ${CMAKE_CURRENT_SOURCE_DIR}/include
#     TARGET_SOURCE_PUBLIC_HEADER
#         ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
#     TARGET_DEPENDENICES_PRIVATE
#         MasterLog
#         ThreadPool
#         ServiceDeclaration
#     TARGET_DEPENDENICES_PUBLIC
#         ServiceExportMacro
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

BuildInterface(
    MODULE_NAME 
        CoreFramework
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PUBLIC
        ServiceExportMacro
        NotificationHelper
        ServiceDeclaration
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    IDE_FOLDER
        framework/Services
)

BuildModule(
        STATIC_LIB
    MODULE_NAME 
        CoreFrameworkImpl
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_DEPENDENICES_PRIVATE
        MasterLog
        ThreadPool
    TARGET_DEPENDENICES_PUBLIC
        CoreFramework
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    IDE_FOLDER
        framework/Services
)

BuildModule(
    MODULE_NAME 
        CoreFrameworkFactory
    TARGET_SOURCE_PRIVATE 
        src/CoreFrameworkFactory.cpp
    TARGET_DEPENDENICES_PRIVATE
        CoreFrameworkImpl
    TARGET_DEPENDENICES_PUBLIC
        CoreFramework
    # TARGET_INCLUDE_DIRECTORIES_PRIVATE
    #     src
    TARGET_DEFINITIONS
        ${ADD_SERVICE_DEFINITIONS}
    IDE_FOLDER
        framework/Services
)

add_subdirectory(test)