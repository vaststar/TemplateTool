#pragma once

#include <memory>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseWrapperExport.h>

namespace ucf::utilities::database {

class IDatabaseWrapper;
struct SqliteDatabaseConfig;
struct MySqlDatabaseConfig;

/**
 * @brief Factory for creating database wrapper instances.
 * 
 * Use this class to create database connections without coupling
 * to specific implementations.
 * 
 * @example
 * @code
 * // Create SQLite database
 * SqliteDatabaseConfig config;
 * config.fileName = "/path/to/db.sqlite";
 * auto db = DatabaseFactory::create(config);
 * db->open();
 * 
 * // Create MySQL database
 * MySqlDatabaseConfig mysqlConfig;
 * mysqlConfig.host = "localhost";
 * mysqlConfig.database = "mydb";
 * mysqlConfig.username = "root";
 * mysqlConfig.password = "secret";
 * auto mysqlDb = DatabaseFactory::create(mysqlConfig);
 * @endcode
 */
class DATABASEWRAPPER_EXPORT DatabaseFactory final {
public:
    DatabaseFactory() = delete;
    
    /**
     * @brief Create a SQLite database wrapper.
     * @param config SQLite configuration
     * @return Shared pointer to database wrapper
     */
    static std::shared_ptr<IDatabaseWrapper> create(const SqliteDatabaseConfig& config);
    
    /**
     * @brief Create a MySQL database wrapper.
     * @param config MySQL configuration
     * @return Shared pointer to database wrapper
     * @note MySQL support requires linking with MySQL client library
     */
    static std::shared_ptr<IDatabaseWrapper> create(const MySqlDatabaseConfig& config);
};

} // namespace ucf::utilities::database
