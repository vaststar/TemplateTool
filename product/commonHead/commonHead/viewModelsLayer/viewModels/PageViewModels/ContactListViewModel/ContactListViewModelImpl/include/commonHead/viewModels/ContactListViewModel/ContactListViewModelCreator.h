#pragma once

#include <memory>

#include <commonHead/viewModels/ContactListViewModel/ContactListViewModelImplExport.h>
#include <commonHead/viewModels/ContactListViewModel/IContactListViewModel.h>

namespace commonHead::viewModels::impl {

CONTACT_LIST_VIEW_MODEL_IMPL_API std::shared_ptr<IContactListViewModel>
createContactListViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

} // namespace commonHead::viewModels::impl
