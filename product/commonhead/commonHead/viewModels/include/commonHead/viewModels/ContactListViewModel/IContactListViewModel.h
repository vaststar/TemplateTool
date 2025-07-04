#pragma once

#include <vector>
#include <memory>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWptr = std::weak_ptr<ICommonHeadFramework>;
}

namespace commonHead::viewModels::model{
    class Contact;
}

namespace commonHead::viewModels{
class COMMONHEAD_EXPORT IContactListViewModelCallback
{
public:
    virtual ~IContactListViewModelCallback() = default;
    virtual void onContactListAvaliable(const std::vector<model::Contact>& contactList) = 0;
};

class COMMONHEAD_EXPORT IContactListViewModel: public virtual commonHead::utilities::IVMNotificationHelper<IContactListViewModelCallback>
{
public:
    virtual std::string getViewModelName() const = 0;
    virtual std::vector<commonHead::viewModels::model::Contact> getContactList() const = 0;
public:
    static std::shared_ptr<IContactListViewModel> createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};
}