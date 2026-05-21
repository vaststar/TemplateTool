include(BuildInterface)

BuildInterface(
    MODULE_NAME
        InfrastructureCommonFile
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        include/ucf/Infrastructure/InfrastructureCommonFile/InfrastructureExport.h
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    IDE_FOLDER
        framework/infrastructure/common_file
)
