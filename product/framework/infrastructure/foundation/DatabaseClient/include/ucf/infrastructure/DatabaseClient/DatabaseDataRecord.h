#pragma once

#include <string>
#include <map>
#include <ucf/infrastructure/DatabaseClient/DatabaseClientExport.h>
#include <ucf/infrastructure/DatabaseClient/DatabaseDataValue.h>

namespace ucf::infrastructure::database{

/**
 * @brief Represents a single row of database query results.
 */
class DATABASE_CLIENT_API DatabaseDataRecord final
{
public:
    void addColumnData(const std::string& key, const DatabaseDataValue& value);
    void addColumnData(const std::string& key, DatabaseDataValue&& value);
    const std::map<std::string, DatabaseDataValue>& getData() const;
private:
    std::map<std::string, DatabaseDataValue> mValueMaps;
};
}
