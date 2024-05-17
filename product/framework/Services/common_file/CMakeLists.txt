include(BuildInterface)

BuildInterface(
    MODULE_NAME 
        ServiceCommonFile
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        include/ServiceCommonFile/ServiceExport.h
        include/ServiceCommonFile/ServiceLogger.h
    TARGET_DEPENDENICES_PUBLIC
        MasterLog
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    # INSTALL_PUBLIC_HEADER 
    #     include/ServiceCommonFile/ServiceExport.h
    #     include/ServiceCommonFile/ServiceLogger.h
)