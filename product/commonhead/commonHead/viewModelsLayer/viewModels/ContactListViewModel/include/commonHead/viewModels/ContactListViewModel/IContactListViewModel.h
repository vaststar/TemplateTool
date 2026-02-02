#pragma once

#include <vector>
#include <memory>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonHead/viewModels/IViewModel/IViewModel.h>

#include <commonHead/viewModels/ContactListViewModel/IContactListModel.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWptr = std::weak_ptr<ICommonHeadFramework>;
}

namespace commonHead::viewModels{
class COMMONHEAD_EXPORT IContactListViewModelCallback
{
public:
    IContactListViewModelCallback() = default;
    IContactListViewModelCallback(const IContactListViewModelCallback&) = delete;
    IContactListViewModelCallback(IContactListViewModelCallback&&) = delete;
    IContactListViewModelCallback& operator=(const IContactListViewModelCallback&) = delete;
    IContactListViewModelCallback& operator=(IContactListViewModelCallback&&) = delete;
    virtual ~IContactListViewModelCallback() = default;
public:
    // virtual void onContactListAvaliable(const std::vector<model::Contact>& contactList) = 0;
};

class COMMONHEAD_EXPORT IContactListViewModel: public IViewModel, public virtual commonHead::utilities::IVMNotificationHelper<IContactListViewModelCallback>
{
public:
    using IViewModel::IViewModel;
    IContactListViewModel(const IContactListViewModel&) = delete;
    IContactListViewModel(IContactListViewModel&&) = delete;
    IContactListViewModel& operator=(const IContactListViewModel&) = delete;
    IContactListViewModel& operator=(IContactListViewModel&&) = delete;
    virtual ~IContactListViewModel() = default;
public:
    virtual std::string getViewModelName() const = 0;
    virtual model::ContactTreePtr getContactList() const = 0;
public:
    static std::shared_ptr<IContactListViewModel> createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};
}