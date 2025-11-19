#pragma once

#include <vector>

namespace ucf::service{
    namespace model{
        class Contact;
    }

class IContactServiceCallback
{
public:
    IContactServiceCallback() = default;
    IContactServiceCallback(const IContactServiceCallback&) = delete;
    IContactServiceCallback(IContactServiceCallback&&) = delete;
    IContactServiceCallback& operator=(const IContactServiceCallback&) = delete;
    IContactServiceCallback& operator=(IContactServiceCallback&&) = delete;
    virtual ~ IContactServiceCallback() = default;
public:
    virtual void OnContactListAvailable(const std::vector<model::Contact>& contactList) = 0;
};
}