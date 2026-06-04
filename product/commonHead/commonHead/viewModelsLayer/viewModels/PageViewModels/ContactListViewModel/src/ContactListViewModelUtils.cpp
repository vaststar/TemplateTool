#include "ContactListViewModelUtils.h"

#include "ContactListModel.h"

namespace commonHead::viewModels::utils {

model::ContactNodeData toVMNodeData(const ucf::service::model::IPersonContactPtr& personContact)
{
    model::ContactNodeData nodeData;
    if (personContact)
    {
        nodeData.id          = personContact->getContactId();
        nodeData.displayName = personContact->getPersonName();
        nodeData.type        = model::ContactNodeType::Person;
    }
    return nodeData;
}

model::ContactNodeData toVMNodeData(const ucf::service::model::IGroupContactPtr& groupContact)
{
    model::ContactNodeData nodeData;
    if (groupContact)
    {
        nodeData.id          = groupContact->getContactId();
        nodeData.displayName = groupContact->getGroupName();
        nodeData.type        = model::ContactNodeType::Group;
    }
    return nodeData;
}

model::ContactRelationData toVMRelation(const ucf::service::model::IContactRelationPtr& serviceRelation)
{
    model::ContactRelationData relationData;
    if (serviceRelation)
    {
        relationData.parentId = serviceRelation->getParentId();
        relationData.childId  = serviceRelation->getChildId();
    }
    return relationData;
}

std::vector<model::ContactNodeData> toVMNodeDatas(const ucf::service::model::PersonContactArray& personContacts)
{
    std::vector<model::ContactNodeData> nodeDatas;
    nodeDatas.reserve(personContacts.size());
    for (const auto& personContact : personContacts)
    {
        if (personContact) { nodeDatas.push_back(toVMNodeData(personContact)); }
    }
    return nodeDatas;
}

std::vector<model::ContactNodeData> toVMNodeDatas(const ucf::service::model::GroupContactArray& groupContacts)
{
    std::vector<model::ContactNodeData> nodeDatas;
    nodeDatas.reserve(groupContacts.size());
    for (const auto& groupContact : groupContacts)
    {
        if (groupContact) { nodeDatas.push_back(toVMNodeData(groupContact)); }
    }
    return nodeDatas;
}

std::vector<model::ContactRelationData> toVMRelations(const ucf::service::model::ContactRelationArray& serviceRelations)
{
    std::vector<model::ContactRelationData> relationDatas;
    relationDatas.reserve(serviceRelations.size());
    for (const auto& serviceRelation : serviceRelations)
    {
        if (serviceRelation) { relationDatas.push_back(toVMRelation(serviceRelation)); }
    }
    return relationDatas;
}

model::ContactDirectoryLoadError toVMLoadError(ucf::service::ContactDirectoryLoadError serviceError)
{
    switch (serviceError)
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

bool isAncestorOf(const std::shared_ptr<model::ContactTree>& contactTree,
                  const std::string& ancestorCandidateId,
                  const std::string& startNodeId)
{
    if (!contactTree || ancestorCandidateId.empty() || startNodeId.empty()) return false;
    auto currentNode = std::static_pointer_cast<model::IContactTree>(contactTree)->findNodeById(startNodeId);
    while (currentNode)
    {
        if (currentNode->getNodeData().id == ancestorCandidateId) return true;
        currentNode = currentNode->getParent().lock();
    }
    return false;
}

VMContactRelation::VMContactRelation(std::string childId, std::string parentId, RelationType relationType)
    : mChildId(std::move(childId)), mParentId(std::move(parentId)), mRelationType(relationType) {}

std::string                     VMContactRelation::getChildId()      const { return mChildId;      }
std::string                     VMContactRelation::getParentId()     const { return mParentId;     }
VMContactRelation::RelationType VMContactRelation::getRelationType() const { return mRelationType; }

} // namespace commonHead::viewModels::utils
