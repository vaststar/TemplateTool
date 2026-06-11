#include "ContactListViewModel.h"

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

void ContactListViewModel::setRelationType(model::RelationType type)
{
    if (mInterestedRelationType == type)
    {
        return;
    }
    COMMONHEAD_LOG_DEBUG("setRelationType from " << static_cast<int>(mInterestedRelationType)
                         << " to " << static_cast<int>(type));
    mInterestedRelationType = type;

    if (auto service = lockService(); service && service->isContactDirectoryReady())
    {
        rebuildTreeFromService();
        fireNotification(&IContactListViewModelCallback::onContactDirectoryReady);
    }
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
    // Register first so we do not miss change events between the isReady probe and the read.
    service->registerCallback(
        std::static_pointer_cast<ucf::service::IContactServiceCallback>(shared_from_this()));

    if (service->isContactDirectoryReady())
    {
        rebuildTreeFromService();
        fireNotification(&IContactListViewModelCallback::onContactDirectoryReady);
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
    // Incremental event arrived before tree was built; rebuild full snapshot instead.
    COMMONHEAD_LOG_DEBUG("ensureTreeBuilt: rebuilding full snapshot");
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

void ContactListViewModel::selectContact(const std::string& contactId)
{
    // Fire-and-forget metrics hook; selection state lives in the UI.
    COMMONHEAD_LOG_INFO("selectContact: contactId:" << (contactId.empty() ? "<cleared>" : contactId));
}

std::optional<model::ContactDetail> ContactListViewModel::getContactDetail(const std::string& contactId) const
{
    if (contactId.empty())
    {
        return std::nullopt;
    }
    auto service = lockService();
    if (!service)
    {
        COMMONHEAD_LOG_ERROR("getContactDetail failed: service not available, id:" << contactId);
        return std::nullopt;
    }

    auto toVMStatus = [](ucf::service::model::IContact::ContactStatus s) {
        return static_cast<model::ContactStatus>(static_cast<int>(s));
    };
    auto toVMGender = [](ucf::service::model::IPersonContact::Gender g) {
        return static_cast<model::Gender>(static_cast<int>(g));
    };
    auto resolvePersonName = [&](const std::string& id) -> std::string {
        if (id.empty()) return {};
        if (auto p = service->getPersonContact(id))
        {
            return p->getPersonName();
        }
        return {};
    };

    if (auto person = service->getPersonContact(contactId))
    {
        model::ContactDetail d;
        d.id          = person->getContactId();
        d.displayName = person->getPersonName();
        d.type        = model::ContactNodeType::Person;
        d.status      = toVMStatus(person->getContactStatus());
        d.person      = model::PersonContactDetail{
            person->getFirstName(),
            person->getLastName(),
            toVMGender(person->getGender()),
            person->getPhone(),
            person->getEmail(),
        };
        return d;
    }

    if (auto group = service->getGroupContact(contactId))
    {
        model::ContactDetail d;
        d.id          = group->getContactId();
        d.displayName = group->getGroupName();
        d.type        = model::ContactNodeType::Group;
        d.groupType   = static_cast<model::GroupType>(static_cast<int>(group->getGroupType()));
        d.status      = toVMStatus(group->getContactStatus());

        // Concrete sub-types fill an extra optional. dynamic_pointer_cast is safe: each
        // typed sub-row is its own interface; missing sub-row simply leaves the optional
        // empty.
        if (auto dept = std::dynamic_pointer_cast<ucf::service::model::IDepartmentGroup>(group))
        {
            d.department = model::DepartmentGroupDetail{
                dept->getManagerId(),
                resolvePersonName(dept->getManagerId()),
                dept->getHeadcount(),
            };
        }
        else if (auto team = std::dynamic_pointer_cast<ucf::service::model::ITeamGroup>(group))
        {
            d.team = model::TeamGroupDetail{
                team->getTeamLeadId(),
                resolvePersonName(team->getTeamLeadId()),
                team->getMission(),
            };
        }
        return d;
    }

    COMMONHEAD_LOG_WARN("getContactDetail: unknown contactId:" << contactId);
    return std::nullopt;
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

    auto curParent = childNode->getParent().lock();
    const std::string curParentId = curParent ? curParent->getNodeData().id : std::string{};
    if (curParentId == newParentId) // noop
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

    // Service-level relation rows only exist when the parent is real (non-empty); the
    // VM tree attaches root-less nodes to a virtual empty-id root. Dispatch accordingly.
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
        // root -> B: no row yet, let the service mint a UUID.
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

// ===== IContactServiceCallback: apply service delta, then forward to subscribers =====

void ContactListViewModel::onContactDirectoryReady()
{
    COMMONHEAD_LOG_DEBUG("onContactDirectoryReady received from service, slice:" << static_cast<int>(mInterestedRelationType));
    rebuildTreeFromService();
    fireNotification(&IContactListViewModelCallback::onContactDirectoryReady);
}

void ContactListViewModel::onContactDirectoryLoadFailed(ucf::service::ContactDirectoryLoadError error)
{
    COMMONHEAD_LOG_ERROR("onContactDirectoryLoadFailed received from service, slice:" << static_cast<int>(mInterestedRelationType)
                         << ", error:" << static_cast<int>(error));
    fireNotification(&IContactListViewModelCallback::onContactDirectoryLoadFailed, utils::toVMLoadError(error));
}

void ContactListViewModel::onPersonContactsAdded(const ucf::service::model::PersonContactArray& persons)
{
    auto vmNodes = utils::toVMNodeDatas(persons);
    COMMONHEAD_LOG_DEBUG("onPersonContactsAdded slice:" << static_cast<int>(mInterestedRelationType)
                         << ", count:" << vmNodes.size());
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
    COMMONHEAD_LOG_DEBUG("onPersonContactsUpdated slice:" << static_cast<int>(mInterestedRelationType)
                         << ", count:" << vmNodes.size());
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
    COMMONHEAD_LOG_DEBUG("onPersonContactsRemoved slice:" << static_cast<int>(mInterestedRelationType)
                         << ", count:" << contactIds.size());
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
    fireNotification(&IContactListViewModelCallback::onPersonContactsRemoved, contactIds);
}

void ContactListViewModel::onGroupContactsAdded(const ucf::service::model::GroupContactArray& groups)
{
    const auto groupType = utils::groupTypeFor(mInterestedRelationType);
    if (!groupType.has_value())
    {
        COMMONHEAD_LOG_DEBUG("onGroupContactsAdded dropped (slice has no group type), slice:"
                             << static_cast<int>(mInterestedRelationType) << ", raw count:" << groups.size());
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
    COMMONHEAD_LOG_DEBUG("onGroupContactsAdded slice:" << static_cast<int>(mInterestedRelationType)
                         << ", raw:" << groups.size() << ", kept:" << filtered.size());
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
        COMMONHEAD_LOG_DEBUG("onGroupContactsUpdated dropped (slice has no group type), slice:"
                             << static_cast<int>(mInterestedRelationType) << ", raw count:" << groups.size());
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
    COMMONHEAD_LOG_DEBUG("onGroupContactsUpdated slice:" << static_cast<int>(mInterestedRelationType)
                         << ", raw:" << groups.size() << ", kept:" << filtered.size());
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
    COMMONHEAD_LOG_DEBUG("onGroupContactsRemoved slice:" << static_cast<int>(mInterestedRelationType)
                         << ", count:" << contactIds.size());
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
    fireNotification(&IContactListViewModelCallback::onGroupContactsRemoved, contactIds);
}

void ContactListViewModel::onContactRelationsAdded(const ucf::service::model::ContactRelationArray& relations)
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
    COMMONHEAD_LOG_DEBUG("onContactRelationsAdded slice:" << static_cast<int>(mInterestedRelationType)
                         << ", raw:" << relations.size() << ", kept:" << filtered.size());
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
    for (const auto& r : vmRelations)
    {
        COMMONHEAD_LOG_DEBUG("  relation+ id:" << r.id << ", parent:" << r.parentId << ", child:" << r.childId);
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
    COMMONHEAD_LOG_DEBUG("onContactRelationsUpdated slice:" << static_cast<int>(mInterestedRelationType)
                         << ", raw:" << relations.size() << ", kept:" << filtered.size());
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
    for (const auto& r : vmRelations)
    {
        COMMONHEAD_LOG_DEBUG("  relation~ id:" << r.id << ", parent:" << r.parentId << ", child:" << r.childId);
    }
    fireNotification(&IContactListViewModelCallback::onContactRelationsUpdated, vmRelations);
}

void ContactListViewModel::onContactRelationsRemoved(const std::vector<std::string>& relationIds)
{
    COMMONHEAD_LOG_DEBUG("onContactRelationsRemoved slice:" << static_cast<int>(mInterestedRelationType)
                         << ", incoming relationIds count:" << relationIds.size());
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onContactRelationsRemoved arrived before tree was built (count:" << relationIds.size()
                            << "); fired onContactDirectoryReady instead of delta");
        fireNotification(&IContactListViewModelCallback::onContactDirectoryReady);
        return;
    }

    std::vector<model::RemovedRelationInfo> removed;
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree)
        {
            removed = mTree->removeRelationsByIds(relationIds);
        }
    }
    COMMONHEAD_LOG_DEBUG("onContactRelationsRemoved matched in this slice:" << removed.size()
                         << " of " << relationIds.size());
    if (removed.empty())
    {
        return;
    }

    std::vector<model::ContactRelationData> payload;
    payload.reserve(removed.size());
    for (auto& r : removed)
    {
        COMMONHEAD_LOG_DEBUG("  relation- id:" << r.relationId << ", oldParent:" << r.oldParentId
                             << ", child:" << r.childId);
        payload.push_back({std::move(r.relationId),
                           std::move(r.oldParentId),
                           std::move(r.childId),
                           mInterestedRelationType});
    }
    fireNotification(&IContactListViewModelCallback::onContactRelationsRemoved, payload);
}

}
