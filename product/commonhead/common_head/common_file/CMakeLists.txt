include(BuildInterface)

BuildInterface(
    MODULE_NAME 
        CommonHeadCommonFile
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        include/commonHead/CommonHeadCommonFile/CommonHeadExport.h
        include/commonHead/CommonHeadCommonFile/CommonHeadLogger.h
    TARGET_DEPENDENICES_PUBLIC
        MasterLog
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
)