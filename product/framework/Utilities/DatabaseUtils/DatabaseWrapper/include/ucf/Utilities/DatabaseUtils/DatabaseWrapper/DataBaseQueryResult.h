#pragma once

#include <string>
#include <variant>
#include <memory>
#include <vector>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseWrapperExport.h>

namespace ucf::utilities::database{

using DatabaseBuffer = std::vector<uint8_t>;
using DatabaseValue = std::variant<bool, int, double, std::string, DatabaseBuffer>;

class DATABASEWRAPPER_EXPORT DataBaseQueryResult final
{
public:
    DataBaseQueryResult();
    ~DataBaseQueryResult();
    void addColumnData(const std::string& key, const DatabaseValue& value);
    DatabaseValue value(const std::string& key) const;
    std::vector<std::string> keys() const;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}