#pragma once

#include <vector>
#include <memory>
#include "CommonHeadCommonFile/CommonHeadExport.h"
#include "NotificationHelper/NotificationHelper.h"


class ICommonHeadFramework;
using ICommonHeadFrameworkWptr = std::weak_ptr<ICommonHeadFramework>;

namespace CommonHead::ViewModels{
class Contact;
class COMMONHEAD_EXPORT IContactListViewModelCallback
{
public:
    virtual ~IContactListViewModelCallback() = default;
    virtual void onContactListAvaliable(const std::vector<Contact>& contactList) = 0;
};

class COMMONHEAD_EXPORT IContactListViewModel: public NotificationHelper<IContactListViewModelCallback>
{
public:
    virtual std::string getViewModelName() const = 0;
    virtual std::vector<CommonHead::ViewModels::Contact> getContactList() const = 0;
public:
    static std::shared_ptr<IContactListViewModel> CreateInstance(ICommonHeadFrameworkWptr commonHeadFramework);
};
}