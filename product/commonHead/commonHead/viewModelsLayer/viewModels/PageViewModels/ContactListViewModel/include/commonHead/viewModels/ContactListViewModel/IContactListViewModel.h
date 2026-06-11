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
    // Carries (relationId, childId, oldParentId, type) for every relation row removed in
    // this slice. Subscribers should re-parent each childId locally (commonly to root in
    // single-parent trees) and may use oldParentId / type for diagnostics or to support
    // multi-parent / edge-precise UIs in the future.
    virtual void onContactRelationsRemoved(const std::vector<model::ContactRelationData>& removed) {}
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

    // Relation slice this VM is bound to (Department / Reporting / ...).
    [[nodiscard]] virtual model::RelationType getRelationType() const = 0;
    virtual void setRelationType(model::RelationType type) = 0;

    // True once the tree has been populated. onContactDirectoryReady will not be re-fired
    // for late subscribers.
    [[nodiscard]] virtual bool isContactDirectoryReady() const = 0;

    // Fire-and-forget metrics hook; VM does not store selection.
    virtual void selectContact(const std::string& contactId) = 0;

    [[nodiscard]] virtual bool canMoveContact(const std::string& childId,
                                              const std::string& newParentId) const = 0;
    virtual void moveContact(const std::string& childId,
                             const std::string& newParentId) = 0;
public:
    static std::shared_ptr<IContactListViewModel> createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};
}
