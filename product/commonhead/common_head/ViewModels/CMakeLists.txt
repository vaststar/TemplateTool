include(BuildModule)

set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC  #for include and install header
include/ContactListViewModel/IContactListViewModel.h
include/ContactListViewModel/ContactListModel.h
)

set(TARGET_SOURCE_FILES_PRIVATE
include/ContactListViewModel/ContactListViewModel.h
src/ContactListViewModel/ContactListViewModel.cpp
src/ContactListViewModel/ContactListModel.cpp
)

BuildModule(
    MODULE_NAME 
        ViewModels
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PRIVATE
        CommonHeadFramework
        ContactService
        CoreFramework
    TARGET_DEPENDENICES_PUBLIC
        CommonHeadCommonFile
        NotificationHelper
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    TARGET_DEFINITIONS
        ${ADD_COMMONHEAD_DEFINITIONS}
    # INSTALL_PUBLIC_HEADER 
    #     include/ContactListViewModel/IContactListViewModel.h  
    #     include/ContactListViewModel/ContactListModel.h 
    # INSTALL_PUBLIC_HEADER_DIR
    #     include/CommonHeadFramework 
    IDE_FOLDER
        commonhead
)