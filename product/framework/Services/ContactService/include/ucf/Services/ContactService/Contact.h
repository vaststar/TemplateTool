#pragma once

#include <string>
#include <vector>

#include <ucf/Services/ServiceCommonFile/ServiceExport.h>

namespace ucf::service::model{
class SERVICE_EXPORT Contact
{
public:
    Contact(const std::string& id, const std::string& name);
    const std::string& getContactId() const;
    
    const std::string& getContactName() const;
    void setContactName(const std::string& name);

    const std::vector<std::string>& getTags() const;
    void setTags(const std::vector<std::string>& tags);
    void addTags(const std::vector<std::string>& tags);
    void removeTags(const std::vector<std::string>& tags);

    bool operator==(const Contact& other);
private:
    std::string mContactId;
    std::string mContactName;
    std::vector<std::string> mContactTags;
};
}