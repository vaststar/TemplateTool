#pragma once
#include "CommonHeadCommonFile/CommonHeadExport.h"

#include <string>
#include <vector>

namespace CommonHead::ViewModels{
class COMMONHEAD_EXPORT Contact
{
public:
    explicit Contact(const std::string& contactId);
    std::string getContactId() const;

    std::string getContactName() const;
    void setContactName(const std::string& name);

    std::string getUpperId() const;
    void setUpperId(const std::string& upperId);

    std::vector<std::string> getLowerIds() const;
    void setLowerIds(const std::vector<std::string>& lowerIds);
private:
    std::string mContactId;
    std::string mContactName;
    std::string mUpperId;
    std::vector<std::string> mLowerIds;
};
}
