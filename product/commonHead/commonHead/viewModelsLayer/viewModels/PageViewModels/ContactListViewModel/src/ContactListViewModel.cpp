#include "ContactListViewModel.h"

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
    if (auto service = lockService())
    {
        service->registerCallback(
            std::static_pointer_cast<ucf::service::IContactServiceCallback>(shared_from_this()));
    }
    rebuildTreeFromService();
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
        return;
    }
    // Pull data from service outside the tree lock (service has its own locks).
    std::vector<ucf::service::model::IContactPtr> allContacts;
    auto allPersonContacts = service->getPersonContactList();
    auto allGroupContacts  = service->getGroupContactList();
    allContacts.insert(allContacts.end(), allPersonContacts.begin(), allPersonContacts.end());
    allContacts.insert(allContacts.end(), allGroupContacts.begin(), allGroupContacts.end());
    auto allRelations = service->getContactRelations();

    auto tree = std::make_shared<model::ContactTree>(allContacts, allRelations);
    {
        std::scoped_lock lk(mTreeMutex);
        mTree = std::move(tree);
    }
}

model::ContactTreePtr ContactListViewModel::getContactList() const
{
    std::scoped_lock lk(mTreeMutex);
    return std::static_pointer_cast<model::IContactTree>(mTree);
}

// ===== IContactServiceCallback: incremental updates =====
//
// Threading note: callbacks are invoked synchronously on the service's worker
// thread. The tree mutex is held only for the local mutation; fireNotification
// is intentionally invoked outside the lock to avoid holding it across user code.

void ContactListViewModel::onContactDirectoryReady()
{
    COMMONHEAD_LOG_DEBUG("ContactListViewModel: contact directory ready");
    rebuildTreeFromService();
    fireNotification(&IContactListViewModelCallback::onContactDirectoryReady);
}

void ContactListViewModel::onPersonContactsAdded(const ucf::service::model::PersonContactArray& persons)
{
    auto vmNodes = toVMNodeDatas(persons);
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->addNodes(vmNodes);
    }
    fireNotification(&IContactListViewModelCallback::onPersonContactsAdded, vmNodes);
}

void ContactListViewModel::onPersonContactsUpdated(const ucf::service::model::PersonContactArray& persons)
{
    auto vmNodes = toVMNodeDatas(persons);
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->updateNodes(vmNodes);
    }
    fireNotification(&IContactListViewModelCallback::onPersonContactsUpdated, vmNodes);
}

void ContactListViewModel::onPersonContactsRemoved(const std::vector<std::string>& contactIds)
{
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->removeNodes(contactIds);
    }
    fireNotification(&IContactListViewModelCallback::onPersonContactsRemoved, contactIds);
}

void ContactListViewModel::onGroupContactsAdded(const ucf::service::model::GroupContactArray& groups)
{
    auto vmNodes = toVMNodeDatas(groups);
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->addNodes(vmNodes);
    }
    fireNotification(&IContactListViewModelCallback::onGroupContactsAdded, vmNodes);
}

void ContactListViewModel::onGroupContactsUpdated(const ucf::service::model::GroupContactArray& groups)
{
    auto vmNodes = toVMNodeDatas(groups);
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->updateNodes(vmNodes);
    }
    fireNotification(&IContactListViewModelCallback::onGroupContactsUpdated, vmNodes);
}

void ContactListViewModel::onGroupContactsRemoved(const std::vector<std::string>& contactIds)
{
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->removeNodes(contactIds);
    }
    fireNotification(&IContactListViewModelCallback::onGroupContactsRemoved, contactIds);
}

void ContactListViewModel::onContactRelationsAdded(const ucf::service::model::ContactRelationArray& relations)
{
    auto vmRelations = toVMRelations(relations);
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
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->clearRelations(childIds);
    }
    fireNotification(&IContactListViewModelCallback::onContactRelationsRemoved, childIds);
}

}
