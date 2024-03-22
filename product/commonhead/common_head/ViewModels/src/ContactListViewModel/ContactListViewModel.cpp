#include "ContactListViewModel/ContactListViewModel.h"
#include "CommonHeadCommonFile/CommonHeadLogger.h"
#include "CommonHeadFramework/ICommonHeadFramework.h"


namespace CommonHead::ViewModels{
std::shared_ptr<IContactListViewModel> IContactListViewModel::CreateInstance(ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<ContactListViewModel>(commonHeadFramework);
}

ContactListViewModel::ContactListViewModel(ICommonHeadFrameworkWptr commonHeadFramework)
    : mCommonHeadFrameworkWptr(commonHeadFramework)
{
     COMMONHEAD_LOG_DEBUG("create ContactListViewModel")
}

std::string ContactListViewModel::getViewModelName() const
{
    return "ContactListViewModel";
}
}