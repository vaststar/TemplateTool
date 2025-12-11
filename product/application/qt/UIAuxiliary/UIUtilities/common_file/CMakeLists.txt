include(BuildInterface)

BuildInterface(
    MODULE_NAME 
        UIUtilitiesCommonFile
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        include/ucf/UIUtilities/UIUtilitiesCommonFile/UIUtilitiesExport.h
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    IDE_FOLDER
        UI
)