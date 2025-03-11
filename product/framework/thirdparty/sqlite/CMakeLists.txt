include(BuildModule)

if (WIN32)
    set(SQLITE_DEFINITIONS SQLITE_BUILD_DLL SQLITE_BUILD_LIB)
else()
    set(SQLITE_DEFINITIONS NO_SQLITE_DEFINITIONS)
endif()

if (MSVC)
add_compile_options(/FIsqlite_export.h)
else()
add_compile_options(-include sqlite_export.h)
endif()

set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC  #for include and install header
src/sqlite3.h
)

set(TARGET_SOURCE_FILES_PRIVATE
src/sqlite3.c
src/sqlite3ext.h
src/sqlite_export.h
)

BuildModule(
    MODULE_NAME 
        Sqlite3
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_LIST_DIR}/src
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        src
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    TARGET_DEFINITIONS
        ${SQLITE_DEFINITIONS}
    INSTALL_PUBLIC_HEADER 
        src/sqlite3.h
    IDE_FOLDER
        thirdparty
)
