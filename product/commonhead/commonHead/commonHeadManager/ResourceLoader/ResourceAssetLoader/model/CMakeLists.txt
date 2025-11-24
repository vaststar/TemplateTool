include(BuildInterface)

BuildInterface(
    MODULE_NAME 
        ResourceAssetLoaderModel
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}
    TARGET_SOURCE_PUBLIC_HEADER
        commonHead/ResourceAssetLoader/model/Asset.h
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        .
    IDE_FOLDER
        commonhead
)