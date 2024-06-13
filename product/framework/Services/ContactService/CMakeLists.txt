include(BuildModule)

set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC  #for include and install header
    include/ucf/Services/ContactService/IContactServiceCallback.h
    include/ucf/Services/ContactService/IContactService.h
    include/ucf/Services/ContactService/ContactModel.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/ContactService.h
    src/ContactService.cpp
    src/ContactModel.cpp
)


BuildModule(
    MODULE_NAME 
        ContactService
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PRIVATE
        Sqlite3
        CoreFramework
        NetworkService
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
    IDE_FOLDER
        framework/Services
)