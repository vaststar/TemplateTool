include(BuildInterface)

BuildInterface(
    MODULE_NAME 
        UtilitiesCommonFile
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        include/ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h
    TARGET_ADD_LINK_LIBRARY_PUBLIC
        MasterLog
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    IDE_FOLDER
        framework/Services
)