#include "ContactListViewModel.h"

#include <algorithm>
#include <mutex>
#include <ucf/Services/ContactService/IContactService.h>
#include <ucf/Services/ContactService/IContactEntities.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>

#include "ContactListModel.h"


namespace commonHead::viewModels{

namespace {

model::ContactNodeData toVMNodeData(const ucf::service::model::IPersonContactPtr& p)
{
    model::ContactNodeData d;
    if (p)
    {
        d.id          = p->getContactId();
        d.displayName = p->getPersonName();
        d.type        = model::ContactNodeType::Person;
    }
    return d;
}

model::ContactNodeData toVMNodeData(const ucf::service::model::IGroupContactPtr& g)
{
    model::ContactNodeData d;
    if (g)
    {
        d.id          = g->getContactId();
        d.displayName = g->getGroupName();
        d.type        = model::ContactNodeType::Group;
    }
    return d;
}

model::ContactRelationData toVMRelation(const ucf::service::model::IContactRelationPtr& r)
{
    model::ContactRelationData d;
    if (r)
    {
        d.parentId = r->getParentId();
        d.childId  = r->getChildId();
    }
    return d;
}

model::ContactDirectoryLoadError toVMLoadError(ucf::service::ContactDirectoryLoadError e)
{
    switch (e)
    {
    case ucf::service::ContactDirectoryLoadError::DatabaseNotBound:
        return model::ContactDirectoryLoadError::DatabaseNotBound;
    case ucf::service::ContactDirectoryLoadError::DatabaseReadFailed:
        return model::ContactDirectoryLoadError::DatabaseReadFailed;
    case ucf::service::ContactDirectoryLoadError::Unknown:
    default:
        return model::ContactDirectoryLoadError::Unknown;
    }
}

std::vector<model::ContactNodeData> toVMNodeDatas(const ucf::service::model::PersonContactArray& persons)
{
    std::vector<model::ContactNodeData> out;
    out.reserve(persons.size());
    for (const auto& p : persons)
    {
        if (p) { out.push_back(toVMNodeData(p)); }
    }
    return out;
}

std::vector<model::ContactNodeData> toVMNodeDatas(const ucf::service::model::GroupContactArray& groups)
{
    std::vector<model::ContactNodeData> out;
    out.reserve(groups.size());
    for (const auto& g : groups)
    {
        if (g) { out.push_back(toVMNodeData(g)); }
    }
    return out;
}

std::vector<model::ContactRelationData> toVMRelations(const ucf::service::model::ContactRelationArray& relations)
{
    std::vector<model::ContactRelationData> out;
    out.reserve(relations.size());
    for (const auto& r : relations)
    {
        if (r) { out.push_back(toVMRelation(r)); }
    }
    return out;
}

} // anonymous namespace

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
    std::vector<ucf::service::model::IContactPtr> allContacts;
    auto allPersonContacts = service->getPersonContactList();
    auto allGroupContacts  = service->getGroupContactList();
    allContacts.insert(allContacts.end(), allPersonContacts.begin(), allPersonContacts.end());
    allContacts.insert(allContacts.end(), allGroupContacts.begin(), allGroupContacts.end());
    auto allRelations = service->getContactRelations();
    COMMONHEAD_LOG_DEBUG("rebuildTreeFromService, persons:" << allPersonContacts.size()
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
    fireNotification(&IContactListViewModelCallback::onContactDirectoryLoadFailed, toVMLoadError(error));
}

void ContactListViewModel::onPersonContactsAdded(const ucf::service::model::PersonContactArray& persons)
{
    auto vmNodes = toVMNodeDatas(persons);
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onPersonContactsAdded arrived before tree was built (count:" << vmNodes.size()
                            << "); fired onContactDirectoryReady instead of delta");
        fireNotification(&IContactListViewModelCallback::onContactDirectoryReady);
        return;
    }
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->addNodes(vmNodes);
    }
    fireNotification(&IContactListViewModelCallback::onPersonContactsAdded, vmNodes);
}

void ContactListViewModel::onPersonContactsUpdated(const ucf::service::model::PersonContactArray& persons)
{
    auto vmNodes = toVMNodeDatas(persons);
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onPersonContactsUpdated arrived before tree was built (count:" << vmNodes.size()
                            << "); fired onContactDirectoryReady instead of delta");
        fireNotification(&IContactListViewModelCallback::onContactDirectoryReady);
        return;
    }
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->updateNodes(vmNodes);
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
        if (mTree) mTree->removeNodes(contactIds);
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
    auto vmNodes = toVMNodeDatas(groups);
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onGroupContactsAdded arrived before tree was built (count:" << vmNodes.size()
                            << "); fired onContactDirectoryReady instead of delta");
        fireNotification(&IContactListViewModelCallback::onContactDirectoryReady);
        return;
    }
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->addNodes(vmNodes);
    }
    fireNotification(&IContactListViewModelCallback::onGroupContactsAdded, vmNodes);
}

void ContactListViewModel::onGroupContactsUpdated(const ucf::service::model::GroupContactArray& groups)
{
    auto vmNodes = toVMNodeDatas(groups);
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onGroupContactsUpdated arrived before tree was built (count:" << vmNodes.size()
                            << "); fired onContactDirectoryReady instead of delta");
        fireNotification(&IContactListViewModelCallback::onContactDirectoryReady);
        return;
    }
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->updateNodes(vmNodes);
    }
    fireNotification(&IContactListViewModelCallback::onGroupContactsUpdated, vmNodes);
}

void ContactListViewModel::onGroupContactsRemoved(const std::vector<std::string>& contactIds)
{
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onGroupContactsRemoved arrived before tree was built (count:" << contactIds.size()
                            << "); fired onContactDirectoryReady instead of delta");
        fireNotification(&IContactListViewModelCallback::onContactDirectoryReady);
        return;
    }
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->removeNodes(contactIds);
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
    auto vmRelations = toVMRelations(relations);
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onContactRelationsAdded arrived before tree was built (count:" << vmRelations.size()
                            << "); fired onContactDirectoryReady instead of delta");
        fireNotification(&IContactListViewModelCallback::onContactDirectoryReady);
        return;
    }
    std::vector<std::pair<std::string, std::string>> pairs;
    pairs.reserve(vmRelations.size());
    for (const auto& r : vmRelations) pairs.emplace_back(r.parentId, r.childId);
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->setRelations(pairs);
    }
    fireNotification(&IContactListViewModelCallback::onContactRelationsAdded, vmRelations);
}

void ContactListViewModel::onContactRelationsUpdated(const ucf::service::model::ContactRelationArray& relations)
{
    auto vmRelations = toVMRelations(relations);
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onContactRelationsUpdated arrived before tree was built (count:" << vmRelations.size()
                            << "); fired onContactDirectoryReady instead of delta");
        fireNotification(&IContactListViewModelCallback::onContactDirectoryReady);
        return;
    }
    std::vector<std::pair<std::string, std::string>> pairs;
    pairs.reserve(vmRelations.size());
    for (const auto& r : vmRelations) pairs.emplace_back(r.parentId, r.childId);
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->setRelations(pairs);
    }
    fireNotification(&IContactListViewModelCallback::onContactRelationsUpdated, vmRelations);
}

void ContactListViewModel::onContactRelationsRemoved(const std::vector<std::string>& childIds)
{
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onContactRelationsRemoved arrived before tree was built (count:" << childIds.size()
                            << "); fired onContactDirectoryReady instead of delta");
        fireNotification(&IContactListViewModelCallback::onContactDirectoryReady);
        return;
    }
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->clearRelations(childIds);
    }
    fireNotification(&IContactListViewModelCallback::onContactRelationsRemoved, childIds);
}

}
