include(BuildModule)

# CameraDirectoryServiceTypes (SHARED) - the service's outward data structures.
# Currently the camera directory entity interfaces (pure-virtual, header-only);
# a stub TU carries an exported anchor so the DLL has an import library.
set(MODULE_MODULE_NAME CameraDirectoryServiceTypes)
set(MODULE_IDE_FOLDER framework/Services)
set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/Services/CameraDirectoryService/ICameraDirectoryEntities.h
)
set(MODULE_SOURCE_PRIVATE
    src/CameraDirectoryTypes.cpp
)
set(MODULE_DEPENDENCIES_PUBLIC
    ServiceExportMacro
    MediaService
)
set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)

BuildModule(
    MODULE_NAME ${MODULE_MODULE_NAME}
    TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
    TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
    TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${MODULE_BUILD_INTERFACE_DIR}
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
    TARGET_DEFINITIONS ${ADD_SERVICE_DEFINITIONS}
    IDE_FOLDER ${MODULE_IDE_FOLDER}
)
