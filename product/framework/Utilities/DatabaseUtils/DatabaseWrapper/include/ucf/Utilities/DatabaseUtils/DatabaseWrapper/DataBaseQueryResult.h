#pragma once

#include <string>
#include <variant>
#include <memory>
#include <vector>
#include <map>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseWrapperExport.h>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseFormatStruct.h>

namespace ucf::utilities::database{

class DBFormatStruct;

class DATABASEWRAPPER_EXPORT DatabaseQueryResult final
{
public:
    DatabaseQueryResult();
    ~DatabaseQueryResult();
    void addColumnData(const std::string& key, const DBFormatStruct& value);
    std::map<std::string, DBFormatStruct> values() const;
    // DBFormatStruct value(const std::string& key) const;
    std::vector<std::string> keys() const;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}