include(BuildInterface)

BuildInterface(
    MODULE_NAME 
        ResourceColorLoaderModel
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}
    TARGET_SOURCE_PUBLIC_HEADER
        commonHead/ResourceColorLoader/model/Color.h
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        .
    IDE_FOLDER
        commonhead
)