include(BuildInterface)

# CameraDirectoryServiceApi (INTERFACE) - pure interfaces + entity interfaces.
set(MODULE_MODULE_NAME CameraDirectoryServiceApi)
set(MODULE_IDE_FOLDER framework/Services)
set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/Services/CameraDirectoryService/ICameraDirectoryService.h
    include/ucf/Services/CameraDirectoryService/ICameraDirectoryServiceCallback.h
    include/ucf/Services/CameraDirectoryService/ICameraDirectoryEntities.h
)
set(MODULE_DEPENDENCIES_PUBLIC
    ServiceDeclaration
    NotificationHelper
    MediaService
)
set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)

BuildInterface(
    MODULE_NAME ${MODULE_MODULE_NAME}
    TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
    TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
    TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${MODULE_BUILD_INTERFACE_DIR}
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
    IDE_FOLDER ${MODULE_IDE_FOLDER}
)
