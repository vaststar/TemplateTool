include(BuildInterface)

BuildInterface(
    MODULE_NAME 
        ServiceCommonFile
    PUBLIC_BUILD_INTERFACE_FOLDER
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC_INSTALL_INTERFACE_FOLDER
        include
    INSTALL_PUBLIC_HEADER 
        include/ServiceCommonFile/ServiceExport.h
        include/ServiceCommonFile/ServiceLogger.h
    IDE_FOLDER
        Services
)