include(BuildModule)

# CameraDirectoryServiceImpl (SHARED) - concrete implementation.
set(MODULE_MODULE_NAME CameraDirectoryServiceImpl)
set(MODULE_IDE_FOLDER framework/Services)
set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/Services/CameraDirectoryService/CameraDirectoryServiceImplExport.h
    include/ucf/Services/CameraDirectoryService/CameraDirectoryServiceCreator.h
)
set(MODULE_SOURCE_PRIVATE
    src/CameraDirectoryServiceLogger.h
    src/CameraDirectoryEntities.h
    src/CameraDirectoryEntities.cpp
    src/CameraDirectoryNotificationSink.h
    src/CameraDirectoryModel.h
    src/CameraDirectoryModel.cpp
    src/CameraDirectoryDBAccess.h
    src/CameraDirectoryDBAccess.cpp
    src/CameraDirectoryManager.h
    src/CameraDirectoryManager.cpp
    src/CameraDirectoryService.h
    src/CameraDirectoryService.cpp
)
set(MODULE_DEPENDENCIES_PUBLIC
    CameraDirectoryServiceApi
)
set(MODULE_DEPENDENCIES_PRIVATE
    DataWarehouseServiceApi
    DataWarehouseSchema
    ClientInfoServiceApi
    CoreFramework
    LoggerUtils
    UUIDUtils
    SinkNotifier
)
set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR src)

set(MODULE_PUBLIC_DEFINITIONS)
set(MODULE_PRIVATE_DEFINITIONS)
if (SERVICE_LIB_TYPE STREQUAL "STATIC_LIB")
    set(MODULE_PUBLIC_DEFINITIONS CAMERA_DIRECTORY_SERVICE_IMPL_STATIC)
else()
    set(MODULE_PRIVATE_DEFINITIONS CAMERA_DIRECTORY_SERVICE_IMPL_SHARED)
endif()

BuildModule(
    ${SERVICE_LIB_TYPE}
    MODULE_NAME ${MODULE_MODULE_NAME}
    TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
    TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
    TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
    TARGET_ADD_LINK_LIBRARY_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${MODULE_BUILD_INTERFACE_DIR}
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
    TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
    TARGET_PUBLIC_DEFINITIONS ${MODULE_PUBLIC_DEFINITIONS}
    TARGET_PRIVATE_DEFINITIONS ${MODULE_PRIVATE_DEFINITIONS}
    IDE_FOLDER ${MODULE_IDE_FOLDER}
)

add_subdirectory(test)
