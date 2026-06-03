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
    virtual void onContactDirectoryReady() {}
    virtual void onContactDirectoryLoadFailed(model::ContactDirectoryLoadError /*error*/) {}
    virtual void onPersonContactsAdded(const std::vector<model::ContactNodeData>& persons) {}
    virtual void onPersonContactsUpdated(const std::vector<model::ContactNodeData>& persons) {}
    virtual void onPersonContactsRemoved(const std::vector<std::string>& contactIds) {}
    virtual void onGroupContactsAdded(const std::vector<model::ContactNodeData>& groups) {}
    virtual void onGroupContactsUpdated(const std::vector<model::ContactNodeData>& groups) {}
    virtual void onGroupContactsRemoved(const std::vector<std::string>& contactIds) {}
    virtual void onContactRelationsAdded(const std::vector<model::ContactRelationData>& relations) {}
    virtual void onContactRelationsUpdated(const std::vector<model::ContactRelationData>& relations) {}
    virtual void onContactRelationsRemoved(const std::vector<std::string>& childIds) {}

    // The VM-owned current selection changed. Empty contactId means selection was cleared
    // (e.g. the previously selected contact was removed). Fired after de-duplication, so
    // subscribers can treat every invocation as a real state change.
    virtual void onCurrentContactChanged(const std::string& /*contactId*/) {}
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

    // True once the VM tree has been populated from a successful service load.
    // Late subscribers should check this on registration; if true, read the tree directly
    // because onContactDirectoryReady will not be re-fired.
    [[nodiscard]] virtual bool isContactDirectoryReady() const = 0;

    // The current selected contact node id (empty if none). VM-owned UI state shared by
    // all subscribers of this VM instance. The underlying ContactService is shared but
    // does not store selection.
    [[nodiscard]] virtual std::string getCurrentContactId() const = 0;

    // ===== Selection =====
    // Set the current selected contact node (Person or Group). Pass an empty string to
    // clear selection. Unknown ids are ignored. Same-as-current calls are deduped.
    // Successful changes fire onCurrentContactChanged; this is the single funnel point
    // for selection metrics / analytics.
    virtual void selectContact(const std::string& contactId) = 0;
public:
    static std::shared_ptr<IContactListViewModel> createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};
}
