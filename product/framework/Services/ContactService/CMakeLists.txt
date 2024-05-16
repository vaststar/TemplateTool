include(BuildModule)

set(target_public_include_headers  #for include and install header
include/ContactService/IContactService.h
include/ContactService/ContactModel.h
)

set(target_build_source_files
include/ContactService/ContactService.h
src/ContactService.cpp
src/ContactModel.cpp
)


BuildModule(
    MODULE_NAME 
        ContactService
    TARGET_SOURCE_PRIVATE 
        ${target_build_source_files}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_LIST_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${target_public_include_headers}
    TARGET_DEPENDENICES_PRIVATE
        Sqlite3
        CoreFramework
    TARGET_DEPENDENICES_PUBLIC
        ServiceCommonFile
        NotificationHelper
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    TARGET_DEFINITIONS
        ${ADD_SERVICE_DEFINITIONS}
    # INSTALL_PUBLIC_HEADER 
    #     include/ContactService/IContactService.h
    #     include/ContactService/ContactModel.h
    # INSTALL_PUBLIC_HEADER_DIR
    #     include/ContactService
    IDE_FOLDER
        framework/Services
)