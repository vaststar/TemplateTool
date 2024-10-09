if (WIN32)
    set(ADD_DATABASEWRAPPER_DEFINITIONS DATABASEWRAPPER_DLL DATABASEWRAPPER_LIB)
endif()

include(BuildModule)

set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC  #for include and install header
    include/ucf/Utilities/DatabaseUtils/DatabaseWrapper/IDatabaseWrapper.h
    include/ucf/Utilities/DatabaseUtils/DatabaseWrapper/DataBaseQueryResult.h
    include/ucf/Utilities/DatabaseUtils/DatabaseWrapper/DataBaseSchema.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/DatabaseWrapperLogger.h
    src/DataBaseQueryResult.cpp
    src/DataBaseSchema.cpp
    src/SqliteWrapper/SqliteDatabaseWrapper.h
    src/SqliteWrapper/SqliteDatabaseWrapper.cpp
)

BuildModule(
    MODULE_NAME 
        DatabaseWrapper
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PUBLIC
        MasterLog
    TARGET_DEPENDENICES_PRIVATE
        UUIDUtils
        Sqlite3
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    TARGET_DEFINITIONS
        ${ADD_DATABASEWRAPPER_DEFINITIONS}
    IDE_FOLDER
        framework/Utilities/DatabaseUtils
)