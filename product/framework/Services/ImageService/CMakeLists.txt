include(BuildUCFServiceModule)

set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC  #for include and install header
    include/ucf/Services/ImageService/IImageService.h
    include/ucf/Services/ImageService/ImageTypes.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/ImageServiceLogger.h
    src/ImageService.h
    src/ImageService.cpp
)

set(TARGET_FACTORY_FILES
    src/ImageServiceFactory.cpp
)

set(TARGET_MODULE_NAME ImageService)

BuildUCFServiceModule(
    MODULE_NAME 
        ${TARGET_MODULE_NAME}
    INTERFACE_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    INTERFACE_FILES
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    INTERFACE_DEPENDENCIES
        ServiceExportMacro
        NotificationHelper
        ServiceDeclaration
    INTERFACE_INSTALL_DIR
        include
    IDE_FOLDER
        framework/Services
    IMPL_FILES
        ${TARGET_SOURCE_FILES_PRIVATE}
    IMPL_DEPENDENCIES_PUBLIC
        CoreFramework
    IMPL_DEPENDENCIES_PRIVATE
        MasterLog
        ThreadPool
    IMPL_BUILD_INCLUDE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    IMPL_BUILD_INCLUDE_DIR_PRIVATE
        src
    DLL_DEFINITIONS
        ${ADD_SERVICE_DEFINITIONS}
    FACTORY_FILES
        ${TARGET_FACTORY_FILES}
)