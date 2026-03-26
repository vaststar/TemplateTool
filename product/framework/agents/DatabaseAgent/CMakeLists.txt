include(BuildModule)

set(MODULE_MODULE_NAME DatabaseAgent)
set(MODULE_IDE_FOLDER framework/Agents)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/Agents/DatabaseAgent/IDatabaseWrapper.h
    include/ucf/Agents/DatabaseAgent/DatabaseConfig.h
    include/ucf/Agents/DatabaseAgent/DatabaseFactory.h
    include/ucf/Agents/DatabaseAgent/DatabaseSchema.h
    include/ucf/Agents/DatabaseAgent/DatabaseDataValue.h
    include/ucf/Agents/DatabaseAgent/DatabaseDataRecord.h
)
set(MODULE_SOURCE_PRIVATE
    src/DatabaseAgentLogger.h
    src/DatabaseFactory.cpp
    src/DatabaseDataRecord.cpp
    src/DatabaseSchema.cpp
    src/DatabaseDataValue.cpp
    src/SqliteWrapper/SqliteDatabaseWrapper.h
    src/SqliteWrapper/SqliteDatabaseWrapper.cpp
)

set(MODULE_DEPENDENCIES_PUBLIC
    AgentsCommonFile
)
set(MODULE_DEPENDENCIES_PRIVATE
    UUIDUtils
    Sqlite3
    LoggerUtils
)

set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR src)

BuildModule(
MODULE_NAME ${MODULE_MODULE_NAME}
TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
TARGET_ADD_LINK_LIBRARY_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE  ${MODULE_BUILD_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
TARGET_DEFINITIONS ${ADD_AGENTS_DEFINITIONS}
IDE_FOLDER ${MODULE_IDE_FOLDER}
)
