include(BuildModule)

set(MODULE_MODULE_NAME DatabaseClient)
set(MODULE_IDE_FOLDER framework/infrastructure/foundation)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/infrastructure/DatabaseClient/DatabaseClientExport.h
    include/ucf/infrastructure/DatabaseClient/IDatabaseWrapper.h
    include/ucf/infrastructure/DatabaseClient/DatabaseConfig.h
    include/ucf/infrastructure/DatabaseClient/DatabaseFactory.h
    include/ucf/infrastructure/DatabaseClient/DatabaseSchema.h
    include/ucf/infrastructure/DatabaseClient/DatabaseDataValue.h
    include/ucf/infrastructure/DatabaseClient/DatabaseDataRecord.h
)
set(MODULE_SOURCE_PRIVATE
    src/DatabaseClientLogger.h
    src/DatabaseFactory.cpp
    src/DatabaseDataRecord.cpp
    src/DatabaseSchema.cpp
    src/DatabaseDataValue.cpp
    src/SqliteWrapper/SqliteDatabaseWrapper.h
    src/SqliteWrapper/SqliteDatabaseWrapper.cpp
)

set(MODULE_DEPENDENCIES_PUBLIC
)
set(MODULE_DEPENDENCIES_PRIVATE
    UUIDUtils
    Sqlite3
    LoggerUtils
)

set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR src)

set(MODULE_PUBLIC_DEFINITIONS)
set(MODULE_PRIVATE_DEFINITIONS)
if (INFRASTRUCTURE_LIB_TYPE STREQUAL "STATIC_LIB")
    set(MODULE_PUBLIC_DEFINITIONS DATABASE_CLIENT_STATIC)
else()
    set(MODULE_PRIVATE_DEFINITIONS DATABASE_CLIENT_SHARED)
endif()

BuildModule(
${INFRASTRUCTURE_LIB_TYPE}
MODULE_NAME ${MODULE_MODULE_NAME}
TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
TARGET_ADD_LINK_LIBRARY_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE  ${MODULE_BUILD_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
TARGET_PUBLIC_DEFINITIONS ${MODULE_PUBLIC_DEFINITIONS}
TARGET_PRIVATE_DEFINITIONS ${MODULE_PRIVATE_DEFINITIONS}
IDE_FOLDER ${MODULE_IDE_FOLDER}
)
