#pragma once

#include <string>
#include <map>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseWrapperExport.h>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseDataValue.h>

namespace ucf::utilities::database{

/**
 * @brief Represents a single row of database query results.
 */
class DATABASEWRAPPER_EXPORT DatabaseDataRecord final
{
public:
    void addColumnData(const std::string& key, const DatabaseDataValue& value);
    void addColumnData(const std::string& key, DatabaseDataValue&& value);
    const std::map<std::string, DatabaseDataValue>& getData() const;
private:
    std::map<std::string, DatabaseDataValue> mValueMaps;
};
}