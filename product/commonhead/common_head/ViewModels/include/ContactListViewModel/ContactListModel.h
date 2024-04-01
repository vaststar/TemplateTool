#pragma once
#include "CommonHeadCommonFile/CommonHeadExport.h"

#include <string>

namespace CommonHead::ViewModels{
class COMMONHEAD_EXPORT Contact
{
public:
    explicit Contact(const std::string& contactId);
    std::string getContactId() const;
private:
    const std::string mContactId;
};
}
