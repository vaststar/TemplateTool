#pragma once

#include <string>

namespace CommonHead::ViewModels{
class Contact
{
public:
    explicit Contact(const std::string& contactId);
    std::string getContactId() const;
private:
    const std::string mContactId;
};
}
