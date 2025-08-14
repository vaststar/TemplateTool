include(BuildModule)

add_subdirectory(resource)
set(MODULE_MODULE_NAME ResourceLoader)
set(MODULE_IDE_FOLDER commonhead)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES #for include and install header
    include/commonHead/ResourceLoader/IResourceLoader.h
    include/commonHead/ResourceLoader/ResourceLoaderModel.h
    include/commonHead/ResourceLoader/IResourceStringLoader.h
)

set(MODULE_SOURCE_PRIVATE
    src/ResourceLoader.h
    src/ResourceLoader.cpp
    src/FontSet.h
    src/FontSet.cpp
    src/ColorSet.h
    src/ColorSet.cpp
    src/ResourceTheme.h
    src/ResourceTheme.cpp
    src/ColorConstant.h
    src/IResourceStringLoader.cpp
    src/ResourceThemeLoader.h
    src/ResourceThemeLoader.cpp
    src/ColorBuilder.h
    src/ColorBuilder.cpp
)
set(MODULE_DEPENDENCIES_PUBLIC
    CommonHeadCommonFile
    ResourceString
)
set(MODULE_DEPENDENCIES_PRIVATE
    ClientInfoService
    CoreFramework
)

set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR} ${CMAKE_BINARY_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR src ${CMAKE_BINARY_DIR})

BuildModule(
MODULE_NAME ${MODULE_MODULE_NAME}
TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
TARGET_DEPENDENCIES_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
TARGET_DEPENDENCIES_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE  ${MODULE_BUILD_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
TARGET_DEFINITIONS ${ADD_COMMONHEAD_DEFINITIONS}
IDE_FOLDER ${MODULE_IDE_FOLDER}
)

