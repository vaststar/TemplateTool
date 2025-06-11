include(BuildModule)

set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC  #for include and install header
    include/commonHead/ServiceLocator/IServiceLocator.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/ServiceLocator.h
    src/ServiceLocator.cpp
)

BuildModule(
    MODULE_NAME 
        ServiceLocator
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PUBLIC
        CommonHeadCommonFile
    TARGET_DEPENDENICES_PRIVATE
        CoreFramework
        DataWarehouseService
        NetworkService
        ClientInfoService
        ContactService
        MediaService
        ImageService
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    TARGET_DEFINITIONS
        ${ADD_COMMONHEAD_DEFINITIONS}
    IDE_FOLDER
        commonhead
)