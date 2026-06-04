#include "ContactListViewModel.h"

#include <algorithm>
#include <mutex>
#include <ucf/Services/ContactService/IContactService.h>
#include <ucf/Services/ContactService/IContactEntities.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>

#include "ContactListModel.h"
#include "ContactListViewModelUtils.h"


namespace commonHead::viewModels{

std::shared_ptr<IContactListViewModel> IContactListViewModel::createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<ContactListViewModel>(commonHeadFramework);
}

ContactListViewModel::ContactListViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : IContactListViewModel(commonHeadFramework)
{
    COMMONHEAD_LOG_DEBUG("create ContactListViewModel");
}

std::string ContactListViewModel::getViewModelName() const
{
    return "ContactListViewModel";
}

model::RelationType ContactListViewModel::getRelationType() const
{
    return mInterestedRelationType;
}

ucf::service::model::IContactRelation::RelationType ContactListViewModel::serviceRelationType() const
{
    return utils::toServiceRelationType(mInterestedRelationType);
}

void ContactListViewModel::init()
{
    auto service = lockService();
    if (!service)
    {
        COMMONHEAD_LOG_ERROR("ContactListViewModel init: service not available");
        return;
    }
    // Register first so we do not miss any change event arriving between the isReady
    // probe and the read below.
    service->registerCallback(
        std::static_pointer_cast<ucf::service::IContactServiceCallback>(shared_from_this()));

    if (service->isContactDirectoryReady())
    {
        COMMONHEAD_LOG_DEBUG("ContactListViewModel init: service already ready, rebuilding tree synchronously");
        rebuildTreeFromService();
        fireNotification(&IContactListViewModelCallback::onContactDirectoryReady);
    }
    else
    {
        // VM does NOT trigger loadContactDirectory(); the Service owns load lifecycle and
        // will call us back through the sink as soon as the DB is bound and loaded.
        COMMONHEAD_LOG_DEBUG("ContactListViewModel init: waiting for service to become ready");
    }
}

std::shared_ptr<ucf::service::IContactService> ContactListViewModel::lockService() const
{
    if (auto framework = getCommonHeadFramework().lock())
    {
        if (auto locator = framework->getServiceLocator())
        {
            return locator->getContactService().lock();
        }
    }
    return nullptr;
}

void ContactListViewModel::rebuildTreeFromService()
{
    auto service = lockService();
    if (!service)
    {
        COMMONHEAD_LOG_ERROR("rebuildTreeFromService skipped: service not available");
        return;
    }
    // Pull only the slice this VM cares about. Persons are common to every slice;
    // groups depend on the slice (Department -> Department groups, Project -> Project
    // groups, Reporting/Mentor/Collaboration -> no groups). The mapping lives in
    // utils::groupTypeFor so adding a new RelationType only requires touching one place.
    const auto relType   = serviceRelationType();
    const auto groupType = utils::groupTypeFor(mInterestedRelationType);

    std::vector<ucf::service::model::IContactPtr> allContacts;
    auto allPersonContacts = service->getPersonContactList();
    allContacts.insert(allContacts.end(), allPersonContacts.begin(), allPersonContacts.end());
    ucf::service::model::GroupContactArray allGroupContacts;
    if (groupType.has_value())
    {
        allGroupContacts = service->getGroupContactList(utils::toServiceGroupType(*groupType));
        allContacts.insert(allContacts.end(), allGroupContacts.begin(), allGroupContacts.end());
    }
    auto allRelations = service->getContactRelations(relType);
    COMMONHEAD_LOG_DEBUG("rebuildTreeFromService, slice relType:" << static_cast<int>(relType)
                         << ", groupType:" << (groupType.has_value() ? static_cast<int>(*groupType) : -1)
                         << ", persons:" << allPersonContacts.size()
                         << ", groups:" << allGroupContacts.size()
                         << ", relations:" << allRelations.size());

    auto tree = std::make_shared<model::ContactTree>(allContacts, allRelations);
    {
        std::scoped_lock lk(mTreeMutex);
        mTree = std::move(tree);
    }
}

bool ContactListViewModel::ensureTreeBuilt()
{
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree)
        {
            return false;
        }
    }
    // Out-of-order safety net: incremental event arrived before our tree was built.
    // Pull a full snapshot now so VM state stays self-consistent; callers surface a Ready
    // notification instead of the original delta (the delta is already in the snapshot).
    COMMONHEAD_LOG_DEBUG("ensureTreeBuilt: tree is null on incremental event, rebuilding full snapshot");
    rebuildTreeFromService();
    return true;
}

model::ContactTreePtr ContactListViewModel::getContactList() const
{
    std::scoped_lock lk(mTreeMutex);
    return std::static_pointer_cast<model::IContactTree>(mTree);
}

bool ContactListViewModel::isContactDirectoryReady() const
{
    std::scoped_lock lk(mTreeMutex);
    return static_cast<bool>(mTree);
}

std::string ContactListViewModel::getCurrentContactId() const
{
    std::scoped_lock lk(mSelectionMutex);
    return mCurrentContactId;
}

void ContactListViewModel::selectContact(const std::string& contactId)
{
    // Empty id is a valid "clear" command; non-empty must resolve to an actual contact
    // node currently in the tree (Person or Group).
    if (!contactId.empty())
    {
        std::shared_ptr<model::ContactTree> treeSnapshot;
        {
            std::scoped_lock lk(mTreeMutex);
            treeSnapshot = mTree;
        }
        if (!treeSnapshot || !treeSnapshot->findNodeById(contactId))
        {
            COMMONHEAD_LOG_WARN("selectContact ignored: not a known contact, contactId:" << contactId);
            return;
        }
    }

    {
        std::scoped_lock lk(mSelectionMutex);
        if (mCurrentContactId == contactId)
        {
            return;
        }
        mCurrentContactId = contactId;
    }
    COMMONHEAD_LOG_INFO("selectContact: contactId:" << (contactId.empty() ? "<cleared>" : contactId));
    fireNotification(&IContactListViewModelCallback::onCurrentContactChanged, contactId);
}

bool ContactListViewModel::canMoveContact(const std::string& childId,
                                          const std::string& newParentId) const
{
    if (childId.empty())
    {
        return false;
    }
    if (childId == newParentId)
    {
        return false;
    }

    std::shared_ptr<model::ContactTree> snapshot;
    {
        std::scoped_lock lk(mTreeMutex);
        snapshot = mTree;
    }
    if (!snapshot)
    {
        return false;
    }

    auto childNode = std::static_pointer_cast<model::IContactTree>(snapshot)->findNodeById(childId);
    if (!childNode)
    {
        return false;
    }

    // Noop if the current parent already matches the target.
    auto curParent = childNode->getParent().lock();
    const std::string curParentId = curParent ? curParent->getNodeData().id : std::string{};
    if (curParentId == newParentId)
    {
        return false;
    }

    if (!newParentId.empty())
    {
        auto parentNode = std::static_pointer_cast<model::IContactTree>(snapshot)->findNodeById(newParentId);
        if (!parentNode)
        {
            return false;
        }
        if (utils::isAncestorOf(snapshot, childId, newParentId))
        {
            return false;
        }
    }
    return true;
}

void ContactListViewModel::moveContact(const std::string& childId,
                                       const std::string& newParentId)
{
    if (!canMoveContact(childId, newParentId))
    {
        COMMONHEAD_LOG_WARN("moveContact rejected: childId=" << childId
                            << ", newParentId=" << (newParentId.empty() ? "<root>" : newParentId));
        return;
    }
    auto service = lockService();
    if (!service)
    {
        COMMONHEAD_LOG_ERROR("moveContact failed: service not available");
        return;
    }

    // Resolve current parent at service level. The VM tree attaches root-less nodes to a
    // virtual root whose id is empty; service-level relation rows only exist when the
    // parent is real (non-empty). So we dispatch to add/update/remove accordingly.
    std::string curParentId;
    {
        std::shared_ptr<model::ContactTree> snapshot;
        {
            std::scoped_lock lk(mTreeMutex);
            snapshot = mTree;
        }
        auto childNode = std::static_pointer_cast<model::IContactTree>(snapshot)->findNodeById(childId);
        if (auto curParent = childNode ? childNode->getParent().lock() : nullptr)
        {
            curParentId = curParent->getNodeData().id;
        }
    }

    COMMONHEAD_LOG_INFO("moveContact: childId=" << childId
                        << ", from=" << (curParentId.empty() ? "<root>" : curParentId)
                        << ", to="   << (newParentId.empty() ? "<root>" : newParentId)
                        << ", relType=" << static_cast<int>(mInterestedRelationType));

    std::string existingRelationId;
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree)
        {
            existingRelationId = mTree->getRelationIdByChildId(childId);
        }
    }

    const auto relType = serviceRelationType();
    if (newParentId.empty())
    {
        if (!existingRelationId.empty())
        {
            service->removeContactRelations({existingRelationId});
        }
        else
        {
            COMMONHEAD_LOG_WARN("moveContact: no existing relation to remove for childId=" << childId
                                << " with empty newParentId; this should not happen");
        }
    }
    else if (existingRelationId.empty())
    {
        // root -> B : no row exists yet; let the service mint a UUID by passing empty id.
        service->addContactRelations({
            std::make_shared<utils::VMContactRelation>(std::string{}, childId, newParentId, relType)
        });
    }
    else
    {
        service->updateContactRelations({
            std::make_shared<utils::VMContactRelation>(existingRelationId, childId, newParentId, relType)
        });
    }
}

// ===== IContactServiceCallback: apply incremental updates then forward to VM subscribers =====

void ContactListViewModel::onContactDirectoryReady()
{
    COMMONHEAD_LOG_DEBUG("onContactDirectoryReady received from service");
    rebuildTreeFromService();
    fireNotification(&IContactListViewModelCallback::onContactDirectoryReady);
}

void ContactListViewModel::onContactDirectoryLoadFailed(ucf::service::ContactDirectoryLoadError error)
{
    COMMONHEAD_LOG_ERROR("onContactDirectoryLoadFailed received from service, error:" << static_cast<int>(error));
    fireNotification(&IContactListViewModelCallback::onContactDirectoryLoadFailed, utils::toVMLoadError(error));
}

void ContactListViewModel::onPersonContactsAdded(const ucf::service::model::PersonContactArray& persons)
{
    auto vmNodes = utils::toVMNodeDatas(persons);
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onPersonContactsAdded arrived before tree was built (count:" << vmNodes.size()
                            << "); fired onContactDirectoryReady instead of delta");
        fireNotification(&IContactListViewModelCallback::onContactDirectoryReady);
        return;
    }
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree)
        {
            mTree->addNodes(vmNodes);
        }
    }
    fireNotification(&IContactListViewModelCallback::onPersonContactsAdded, vmNodes);
}

void ContactListViewModel::onPersonContactsUpdated(const ucf::service::model::PersonContactArray& persons)
{
    auto vmNodes = utils::toVMNodeDatas(persons);
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onPersonContactsUpdated arrived before tree was built (count:" << vmNodes.size()
                            << "); fired onContactDirectoryReady instead of delta");
        fireNotification(&IContactListViewModelCallback::onContactDirectoryReady);
        return;
    }
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree)
        {
            mTree->updateNodes(vmNodes);
        }
    }
    fireNotification(&IContactListViewModelCallback::onPersonContactsUpdated, vmNodes);
}

void ContactListViewModel::onPersonContactsRemoved(const std::vector<std::string>& contactIds)
{
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onPersonContactsRemoved arrived before tree was built (count:" << contactIds.size()
                            << "); fired onContactDirectoryReady instead of delta");
        fireNotification(&IContactListViewModelCallback::onContactDirectoryReady);
        return;
    }
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree)
        {
            mTree->removeNodes(contactIds);
        }
    }
    bool clearedSelection = false;
    {
        std::scoped_lock lk(mSelectionMutex);
        if (!mCurrentContactId.empty()
            && std::find(contactIds.begin(), contactIds.end(), mCurrentContactId) != contactIds.end())
        {
            mCurrentContactId.clear();
            clearedSelection = true;
        }
    }
    fireNotification(&IContactListViewModelCallback::onPersonContactsRemoved, contactIds);
    if (clearedSelection)
    {
        COMMONHEAD_LOG_INFO("selectContact auto-cleared: selected person was removed");
        fireNotification(&IContactListViewModelCallback::onCurrentContactChanged, std::string{});
    }
}

void ContactListViewModel::onGroupContactsAdded(const ucf::service::model::GroupContactArray& groups)
{
    // Filter to this VM's GroupType slice. Slices whose parent is a Person (Reporting /
    // Mentor / Collaboration) declare std::nullopt and have no groups at all in the
    // tree, so we drop the whole event.
    const auto groupType = utils::groupTypeFor(mInterestedRelationType);
    if (!groupType.has_value())
    {
        return;
    }
    const auto wantedGroupType = utils::toServiceGroupType(*groupType);

    ucf::service::model::GroupContactArray filtered;
    filtered.reserve(groups.size());
    for (const auto& g : groups)
    {
        if (g && g->getGroupType() == wantedGroupType)
        {
            filtered.push_back(g);
        }
    }
    if (filtered.empty())
    {
        return;
    }

    auto vmNodes = utils::toVMNodeDatas(filtered);
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onGroupContactsAdded arrived before tree was built (count:" << vmNodes.size()
                            << "); fired onContactDirectoryReady instead of delta");
        fireNotification(&IContactListViewModelCallback::onContactDirectoryReady);
        return;
    }
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree)
        {
            mTree->addNodes(vmNodes);
        }
    }
    fireNotification(&IContactListViewModelCallback::onGroupContactsAdded, vmNodes);
}

void ContactListViewModel::onGroupContactsUpdated(const ucf::service::model::GroupContactArray& groups)
{
    const auto groupType = utils::groupTypeFor(mInterestedRelationType);
    if (!groupType.has_value())
    {
        return;
    }
    const auto wantedGroupType = utils::toServiceGroupType(*groupType);

    ucf::service::model::GroupContactArray filtered;
    filtered.reserve(groups.size());
    for (const auto& g : groups)
    {
        if (g && g->getGroupType() == wantedGroupType)
        {
            filtered.push_back(g);
        }
    }
    if (filtered.empty())
    {
        return;
    }

    auto vmNodes = utils::toVMNodeDatas(filtered);
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onGroupContactsUpdated arrived before tree was built (count:" << vmNodes.size()
                            << "); fired onContactDirectoryReady instead of delta");
        fireNotification(&IContactListViewModelCallback::onContactDirectoryReady);
        return;
    }
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree)
        {
            mTree->updateNodes(vmNodes);
        }
    }
    fireNotification(&IContactListViewModelCallback::onGroupContactsUpdated, vmNodes);
}

void ContactListViewModel::onGroupContactsRemoved(const std::vector<std::string>& contactIds)
{
    // Removed events do not carry the GroupType, so we apply optimistically: removing a
    // node id that is not in our tree is a no-op inside ContactTree, which is safe.
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onGroupContactsRemoved arrived before tree was built (count:" << contactIds.size()
                            << "); fired onContactDirectoryReady instead of delta");
        fireNotification(&IContactListViewModelCallback::onContactDirectoryReady);
        return;
    }
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree)
        {
            mTree->removeNodes(contactIds);
        }
    }
    bool clearedSelection = false;
    {
        std::scoped_lock lk(mSelectionMutex);
        if (!mCurrentContactId.empty()
            && std::find(contactIds.begin(), contactIds.end(), mCurrentContactId) != contactIds.end())
        {
            mCurrentContactId.clear();
            clearedSelection = true;
        }
    }
    fireNotification(&IContactListViewModelCallback::onGroupContactsRemoved, contactIds);
    if (clearedSelection)
    {
        COMMONHEAD_LOG_INFO("selectContact auto-cleared: selected group was removed");
        fireNotification(&IContactListViewModelCallback::onCurrentContactChanged, std::string{});
    }
}

void ContactListViewModel::onContactRelationsAdded(const ucf::service::model::ContactRelationArray& relations)
{
    // Only relations of this VM's RelationType slice are interesting; others belong to
    // sibling VM instances (Reporting, Project, ...).
    const auto wanted = serviceRelationType();
    ucf::service::model::ContactRelationArray filtered;
    filtered.reserve(relations.size());
    for (const auto& r : relations)
    {
        if (r && r->getRelationType() == wanted)
        {
            filtered.push_back(r);
        }
    }
    if (filtered.empty())
    {
        return;
    }

    auto vmRelations = utils::toVMRelations(filtered);
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onContactRelationsAdded arrived before tree was built (count:" << vmRelations.size()
                            << "); fired onContactDirectoryReady instead of delta");
        fireNotification(&IContactListViewModelCallback::onContactDirectoryReady);
        return;
    }
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree)
        {
            mTree->setRelations(vmRelations);
        }
    }
    fireNotification(&IContactListViewModelCallback::onContactRelationsAdded, vmRelations);
}

void ContactListViewModel::onContactRelationsUpdated(const ucf::service::model::ContactRelationArray& relations)
{
    const auto wanted = serviceRelationType();
    ucf::service::model::ContactRelationArray filtered;
    filtered.reserve(relations.size());
    for (const auto& r : relations)
    {
        if (r && r->getRelationType() == wanted)
        {
            filtered.push_back(r);
        }
    }
    if (filtered.empty())
    {
        return;
    }

    auto vmRelations = utils::toVMRelations(filtered);
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onContactRelationsUpdated arrived before tree was built (count:" << vmRelations.size()
                            << "); fired onContactDirectoryReady instead of delta");
        fireNotification(&IContactListViewModelCallback::onContactDirectoryReady);
        return;
    }
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree)
        {
            mTree->setRelations(vmRelations);
        }
    }
    fireNotification(&IContactListViewModelCallback::onContactRelationsUpdated, vmRelations);
}

void ContactListViewModel::onContactRelationsRemoved(const std::vector<std::string>& relationIds)
{
    // Service emits a flat list of relationIds covering every slice. Ask the tree to
    // translate each id back to the childId that belongs to this slice; unknown ids
    // (those owned by sibling VMs) are skipped automatically.
    std::vector<std::string> childIds;
    std::vector<std::string> matchedRelationIds;
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree)
        {
            for (const auto& rid : relationIds)
            {
                auto childId = mTree->takeChildIdByRelationId(rid);
                if (childId.empty())
                {
                    continue;
                }
                childIds.push_back(std::move(childId));
                matchedRelationIds.push_back(rid);
            }
            mTree->clearRelations(childIds);
        }
    }
    if (childIds.empty())
    {
        return;
    }

    fireNotification(&IContactListViewModelCallback::onContactRelationsRemoved, matchedRelationIds);
}

}
