#pragma once

#include <string>
#include <functional>
#include <memory>


namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service::model{
    class Contact;
}
namespace ucf::adapter{

using fetchContactInfoCallBack = std::function<void(const ucf::service::model::Contact&)>;
class ContactAdapter
{
public:
    explicit ContactAdapter(ucf::framework::ICoreFrameworkWPtr coreframework);
    ~ContactAdapter();
    void fetchContactInfo(const std::string& contactId, fetchContactInfoCallBack);
private:
    void testFunc();
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}