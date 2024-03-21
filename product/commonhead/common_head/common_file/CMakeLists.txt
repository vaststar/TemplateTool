include(BuildInterface)

BuildInterface(
    MODULE_NAME 
        CommonHeadCommonFile
    PUBLIC_BUILD_INTERFACE_FOLDER
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC_INSTALL_INTERFACE_FOLDER
        include
    INSTALL_PUBLIC_HEADER 
        include/CommonHeadCommonFile/CommonHeadExport.h
        include/CommonHeadCommonFile/CommonHeadLogger.h
    IDE_FOLDER
        CommonHeads
)