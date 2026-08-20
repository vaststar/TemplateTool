cmake_minimum_required(VERSION 3.25)

project(Sqlite3Vendor LANGUAGES C)

set(sqlite_target Sqlite3)
set(sqlite_source_dir "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_library("${sqlite_target}" SHARED)
add_library(Sqlite3::Sqlite3 ALIAS "${sqlite_target}")

target_sources("${sqlite_target}"
    PRIVATE
        "${sqlite_source_dir}/sqlite3.c"
        "${sqlite_source_dir}/sqlite3ext.h"
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            "${sqlite_source_dir}"
        FILES
            "${sqlite_source_dir}/sqlite3.h"
)

target_include_directories("${sqlite_target}"
    PUBLIC
        "$<BUILD_INTERFACE:${sqlite_source_dir}>"
        "$<INSTALL_INTERFACE:include>"
)

# sqlite3.h intentionally allows callers to define SQLITE_API. Keep the
# vendor sources untouched and express the Windows DLL boundary on the target.
target_compile_definitions("${sqlite_target}"
    PRIVATE
        "$<$<PLATFORM_ID:Windows>:SQLITE_API=__declspec(dllexport)>"
    INTERFACE
        "$<$<PLATFORM_ID:Windows>:SQLITE_API=__declspec(dllimport)>"
)

set_target_properties("${sqlite_target}" PROPERTIES
    FOLDER "framework/thirdparty/sqlite"
)

if(APPLE)
    set_target_properties("${sqlite_target}" PROPERTIES
        INSTALL_NAME_DIR "@rpath"
        INSTALL_RPATH "@loader_path;@executable_path"
        BUILD_WITH_INSTALL_RPATH OFF
        INSTALL_RPATH_USE_LINK_PATH OFF
    )
elseif(UNIX)
    set_target_properties("${sqlite_target}" PROPERTIES
        INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib"
        BUILD_WITH_INSTALL_RPATH OFF
        INSTALL_RPATH_USE_LINK_PATH OFF
    )
endif()

# Preserve the project's existing package layout while keeping the vendor
# target independent from TemplateTool's BuildModule/BuildInstallModule.
if(UNIX)
    set(sqlite_library_destination bin)
else()
    set(sqlite_library_destination lib)
endif()

install(TARGETS "${sqlite_target}"
    EXPORT Sqlite3Targets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION "${sqlite_library_destination}"
    ARCHIVE DESTINATION lib
    FILE_SET HEADERS DESTINATION include
    INCLUDES DESTINATION include
)

install(EXPORT Sqlite3Targets
    FILE Sqlite3Config.cmake
    DESTINATION cmake/Sqlite3
    NAMESPACE Sqlite3::
)
