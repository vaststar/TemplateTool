include(BuildModule)

set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC  #for include and install header
    include/commonHead/CommonHeadFramework/ICommonHeadFramework.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/CommonHeadFramework.h
    src/CommonHeadFramework.cpp
)

BuildModule(
    MODULE_NAME 
        CommonHeadFramework
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PRIVATE
        CoreFramework
        ResourceLoader
        ServiceLocator
    TARGET_DEPENDENICES_PUBLIC
        CommonHeadCommonFile
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