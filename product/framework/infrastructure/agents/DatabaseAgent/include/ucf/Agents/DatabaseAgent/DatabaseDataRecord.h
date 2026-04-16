#pragma once

#include <string>
#include <map>
#include <ucf/Agents/AgentsCommonFile/AgentsExport.h>
#include <ucf/Agents/DatabaseAgent/DatabaseDataValue.h>

namespace ucf::agents::database{

/**
 * @brief Represents a single row of database query results.
 */
class Agents_EXPORT DatabaseDataRecord final
{
public:
    void addColumnData(const std::string& key, const DatabaseDataValue& value);
    void addColumnData(const std::string& key, DatabaseDataValue&& value);
    const std::map<std::string, DatabaseDataValue>& getData() const;
private:
    std::map<std::string, DatabaseDataValue> mValueMaps;
};
}
