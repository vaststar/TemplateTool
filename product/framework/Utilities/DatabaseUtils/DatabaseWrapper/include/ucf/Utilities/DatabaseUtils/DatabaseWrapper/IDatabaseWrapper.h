#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <source_location>

#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseWrapperExport.h>

namespace ucf::utilities::database{

class DatabaseDataRecord;
using DatabaseDataRecords = std::vector<DatabaseDataRecord>;
using DatabaseDataRecordsCallback = std::function<void(const DatabaseDataRecords&)>;

class DatabaseSchema;
using DatabaseSchemas = std::vector<DatabaseSchema>;

using Columns = std::vector<std::string>;
 
class DatabaseDataValue;
using Arguments = std::vector<DatabaseDataValue>;
using ListOfArguments = std::vector<Arguments>;

/**
 * @brief Comparison operators for WHERE conditions.
 */
enum class DBOperatorType
{
    Equal,      ///< column = value
    Less,       ///< column < value
    Greater,    ///< column > value
    Match,      ///< column MATCH value (FTS)
    In,         ///< column IN (values)
    NotIn,      ///< column NOT IN (values)
    And,        ///< column & value (bitwise)
    Like,       ///< column LIKE value
    Not,        ///< column <> value
    IsNull      ///< column IS NULL
};

using WhereCondition = std::tuple<std::string, DatabaseDataValue, DBOperatorType>;
using ListsOfWhereCondition = std::vector<WhereCondition>;

/**
 * @brief Abstract interface for database operations.
 * 
 * This interface provides a unified API for different database backends.
 * Use DatabaseFactory to create instances of specific implementations.
 * 
 * @see DatabaseFactory
 */
class DATABASEWRAPPER_EXPORT IDatabaseWrapper
{
public:
    IDatabaseWrapper() = default;
    IDatabaseWrapper(const IDatabaseWrapper&) = delete;
    IDatabaseWrapper(IDatabaseWrapper&&) = delete;
    IDatabaseWrapper& operator=(const IDatabaseWrapper&) = delete;
    IDatabaseWrapper& operator=(IDatabaseWrapper&&) = delete;
    virtual ~IDatabaseWrapper() = default;

public:
    /// @name Connection Management
    /// @{
    
    /**
     * @brief Open the database connection.
     * @return true if opened successfully, false otherwise.
     */
    virtual bool open() = 0;
    virtual void close() = 0;
    virtual bool isOpen() = 0;
    /// @}

    /// @name Schema Operations
    /// @{
    virtual void createTables(const DatabaseSchemas& tableSchemas) = 0;
    /// @}

    /// @name CRUD Operations
    /// @{
    
    /**
     * @brief Insert a single row into the database.
     * 
     * @param tableName The target table name.
     * @param columns Column names.
     * @param arguments Single row data to insert.
     * @param location Source location for debugging.
     * @return true if insert succeeded, false otherwise.
     */
    virtual bool insertIntoDatabase(
        const std::string& tableName, 
        const Columns& columns, 
        const Arguments& arguments, 
        const std::source_location location = std::source_location::current()) = 0;
    
    /**
     * @brief Insert multiple rows atomically (batch operation).
     * 
     * All rows are inserted within a transaction. If any insert fails,
     * all changes are rolled back.
     * 
     * @param tableName The target table name.
     * @param columns Column names.
     * @param arguments List of row data to insert.
     * @param location Source location for debugging.
     * @return true if all inserts succeeded and committed, false otherwise.
     */
    virtual bool insertBatch(
        const std::string& tableName, 
        const Columns& columns, 
        const ListOfArguments& arguments, 
        const std::source_location location = std::source_location::current()) = 0;
    
    /**
     * @brief Fetch rows from database.
     * 
     * @param tableName The target table name.
     * @param columns Column names to retrieve (empty = all columns).
     * @param conditions WHERE conditions.
     * @param callback Callback to receive results.
     * @param limit Maximum rows to return (0 = no limit).
     * @param location Source location for debugging.
     */
    virtual void fetchFromDatabase(
        const std::string& tableName, 
        const Columns& columns, 
        const ListsOfWhereCondition& conditions, 
        DatabaseDataRecordsCallback callback, 
        size_t limit = 0, 
        const std::source_location location = std::source_location::current()) = 0;
    
    /**
     * @brief Update rows matching conditions.
     * 
     * @param tableName The target table name.
     * @param columns Column names to update.
     * @param values New values for the columns.
     * @param conditions WHERE conditions to match rows.
     * @param location Source location for debugging.
     * @return Number of rows updated, or -1 on error.
     */
    virtual int64_t updateInDatabase(
        const std::string& tableName,
        const Columns& columns,
        const Arguments& values,
        const ListsOfWhereCondition& conditions,
        const std::source_location location = std::source_location::current()) = 0;
    
    /**
     * @brief Update multiple rows with different values atomically.
     * 
     * All updates are wrapped in a transaction. If any update fails,
     * all changes are rolled back.
     * 
     * @param tableName The target table name.
     * @param keyColumns Columns for WHERE clause (typically primary key).
     * @param valueColumns Columns to update (SET clause).
     * @param items Each row contains [keyValues..., valueValues...].
     * @param location Source location for debugging.
     * @return true if all updates succeeded and committed, false otherwise.
     * 
     * @example
     * updateBatch("users", {"id"}, {"name", "age"}, 
     *             {{1, "Alice", 30}, {2, "Bob", 25}});
     * // Executes atomically:
     * // UPDATE users SET name='Alice', age=30 WHERE id=1
     * // UPDATE users SET name='Bob', age=25 WHERE id=2
     */
    virtual bool updateBatch(
        const std::string& tableName,
        const Columns& keyColumns,
        const Columns& valueColumns,
        const ListOfArguments& items,
        const std::source_location location = std::source_location::current()) = 0;
    
    /**
     * @brief Delete rows matching conditions.
     * 
     * @param tableName The target table name.
     * @param conditions WHERE conditions to match rows.
     * @param location Source location for debugging.
     * @return Number of rows deleted, or -1 on error.
     */
    virtual int64_t deleteFromDatabase(
        const std::string& tableName,
        const ListsOfWhereCondition& conditions,
        const std::source_location location = std::source_location::current()) = 0;
    /// @}

    /// @name Utility Operations
    /// @{
    
    /**
     * @brief Check if any record matches conditions.
     * 
     * @param tableName The target table name.
     * @param conditions WHERE conditions to match.
     * @return true if at least one matching record exists.
     */
    virtual bool exists(
        const std::string& tableName,
        const ListsOfWhereCondition& conditions) = 0;
    
    /**
     * @brief Count records matching conditions.
     * 
     * @param tableName The target table name.
     * @param conditions WHERE conditions (empty = count all rows).
     * @return Number of matching rows, or -1 on error.
     */
    virtual int64_t count(
        const std::string& tableName,
        const ListsOfWhereCondition& conditions) = 0;
    /// @}
};

} // namespace ucf::utilities::database