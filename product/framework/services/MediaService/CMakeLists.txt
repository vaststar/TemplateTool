include(BuildModule)

set(MODULE_MODULE_NAME MediaService)
set(MODULE_IDE_FOLDER framework/services)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES #for include and install header
    include/ucf/services/MediaService/MediaServiceExport.h
    include/ucf/services/MediaService/IMediaService.h
    include/ucf/services/MediaService/IMediaServiceCallback.h
    include/ucf/services/MediaService/MediaTypes.h
)
set(MODULE_SOURCE_PRIVATE
    src/MediaServiceLogger.h
    src/MediaService.h
    src/MediaService.cpp
    src/CameraDevice.h
    src/CameraDevice.cpp
    src/CameraManager.h
    src/CameraManager.cpp
    src/CameraVideoCapture.h
    src/CameraVideoCapture.cpp
    src/VideoFrame.h
    src/VideoFrame.cpp
)
set(MODULE_DEPENDENCIES_PUBLIC
    NotificationHelper
    ServiceDeclaration
)
set(MODULE_DEPENDENCIES_PRIVATE
    CoreFrameworkApi
    UUIDUtils
    OSUtils
    LoggerUtils
    opencv
)

set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR src)

set(MODULE_PUBLIC_DEFINITIONS)
set(MODULE_PRIVATE_DEFINITIONS)
if (SERVICE_LIB_TYPE STREQUAL "STATIC_LIB")
    set(MODULE_PUBLIC_DEFINITIONS MEDIA_SERVICE_STATIC)
else()
    set(MODULE_PRIVATE_DEFINITIONS MEDIA_SERVICE_SHARED)
endif()

BuildModule(
${SERVICE_LIB_TYPE}
MODULE_NAME ${MODULE_MODULE_NAME}
TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
TARGET_ADD_LINK_LIBRARY_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE  ${MODULE_BUILD_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
TARGET_PUBLIC_DEFINITIONS ${MODULE_PUBLIC_DEFINITIONS}
TARGET_PRIVATE_DEFINITIONS ${MODULE_PRIVATE_DEFINITIONS}
IDE_FOLDER ${MODULE_IDE_FOLDER}
)

add_subdirectory(test)

