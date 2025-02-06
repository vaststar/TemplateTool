#pragma once

#include <vector>
#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

namespace ucf::service{
    namespace model{
        class Contact;
    }

class SERVICE_EXPORT IContactServiceCallback
{
public:
    virtual ~ IContactServiceCallback() = default;
    virtual void OnContactListAvailable(const std::vector<model::Contact>& contactList) = 0;
};
}