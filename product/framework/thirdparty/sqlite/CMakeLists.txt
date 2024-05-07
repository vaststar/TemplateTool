include(BuildModule)

if (WIN32)
    set(SQLITE_DEFINITIONS SQLITE_BUILD_DLL SQLITE_BUILD_LIB)
endif()

if (MSVC)
add_compile_options(/FIsqlite_export.h)
else()
add_compile_options()
endif(-include sqlite_export.h)

BuildModule(
    MODULE_NAME 
        Sqlite3
    TARGET_SOURCE 
        src/sqlite3.c
        src/sqlite3.h
        src/sqlite3ext.h
        src/sqlite_export.h
    PUBLIC_BUILD_INTERFACE_FOLDER
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC_INSTALL_INTERFACE_FOLDER
        src
    PRIVATE_BUILD_FOLDER
        src
    TARGET_DEFINITIONS
        ${SQLITE_DEFINITIONS}
    INSTALL_PUBLIC_HEADER 
        src/sqlite3.h
    IDE_FOLDER
        thirdparty
)
