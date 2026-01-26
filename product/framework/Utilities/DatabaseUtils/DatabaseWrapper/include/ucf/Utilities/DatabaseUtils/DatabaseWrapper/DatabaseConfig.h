#pragma once

#include <string>
#include <cstdint>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseWrapperExport.h>

namespace ucf::utilities::database {

/**
 * @brief Supported database types.
 */
enum class DatabaseType {
    SQLite,
    MySQL,
    PostgreSQL  // Reserved for future
};

/**
 * @brief SQLite database configuration.
 * 
 * @example
 * @code
 * SqliteDatabaseConfig config;
 * config.fileName = "/path/to/database.db";
 * config.password = "optional_encryption_key";  // For SQLCipher
 * @endcode
 */
struct DATABASEWRAPPER_EXPORT SqliteDatabaseConfig {
    std::string fileName;               ///< Path to the SQLite database file
    std::string password;               ///< Encryption password (requires SQLCipher)
    bool createIfNotExists = true;      ///< Create file if it doesn't exist
};

/**
 * @brief MySQL database configuration.
 * 
 * @example
 * @code
 * MySqlDatabaseConfig config;
 * config.host = "localhost";
 * config.port = 3306;
 * config.database = "mydb";
 * config.username = "root";
 * config.password = "secret";
 * @endcode
 */
struct DATABASEWRAPPER_EXPORT MySqlDatabaseConfig {
    std::string host = "localhost";     ///< Server hostname or IP
    uint16_t port = 3306;               ///< Server port
    std::string database;               ///< Database name
    std::string username;               ///< Login username
    std::string password;               ///< Login password
    std::string charset = "utf8mb4";    ///< Character set
    uint32_t connectTimeoutSecs = 10;   ///< Connection timeout
    bool autoReconnect = true;          ///< Auto reconnect on connection loss
};

} // namespace ucf::utilities::database
