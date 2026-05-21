#pragma once

#include <string>
#include <map>
#include <ucf/Infrastructure/InfrastructureCommonFile/InfrastructureExport.h>
#include <ucf/Infrastructure/DatabaseClient/DatabaseDataValue.h>

namespace ucf::infrastructure::database{

/**
 * @brief Represents a single row of database query results.
 */
class Infrastructure_EXPORT DatabaseDataRecord final
{
public:
    void addColumnData(const std::string& key, const DatabaseDataValue& value);
    void addColumnData(const std::string& key, DatabaseDataValue&& value);
    const std::map<std::string, DatabaseDataValue>& getData() const;
private:
    std::map<std::string, DatabaseDataValue> mValueMaps;
};
}
