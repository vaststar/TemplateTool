#pragma once

#include <vector>

namespace ucf::service{
    namespace model{
        class Contact;
    }

class IContactServiceCallback
{
public:
    virtual ~ IContactServiceCallback() = default;
    virtual void OnContactListAvailable(const std::vector<model::Contact>& contactList) = 0;
};
}