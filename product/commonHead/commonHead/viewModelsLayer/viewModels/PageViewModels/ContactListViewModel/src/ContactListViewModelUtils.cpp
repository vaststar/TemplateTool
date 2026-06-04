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
        nodeData.groupType   = toVMGroupType(groupContact->getGroupType());
    }
    return nodeData;
}

model::ContactRelationData toVMRelation(const ucf::service::model::IContactRelationPtr& serviceRelation)
{
    model::ContactRelationData relationData;
    if (serviceRelation)
    {
        relationData.id       = serviceRelation->getRelationId();
        relationData.parentId = serviceRelation->getParentId();
        relationData.childId  = serviceRelation->getChildId();
        relationData.type     = toVMRelationType(serviceRelation->getRelationType());
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

ucf::service::model::IContactRelation::RelationType toServiceRelationType(model::RelationType type)
{
    using S = ucf::service::model::IContactRelation::RelationType;
    switch (type)
    {
    case model::RelationType::Department:    return S::Department;
    case model::RelationType::Reporting:     return S::Reporting;
    case model::RelationType::Project:       return S::Project;
    case model::RelationType::Mentor:        return S::Mentor;
    case model::RelationType::Collaboration: return S::Collaboration;
    }
    return S::Department;
}

model::RelationType toVMRelationType(ucf::service::model::IContactRelation::RelationType type)
{
    using S = ucf::service::model::IContactRelation::RelationType;
    switch (type)
    {
    case S::Department:    return model::RelationType::Department;
    case S::Reporting:     return model::RelationType::Reporting;
    case S::Project:       return model::RelationType::Project;
    case S::Mentor:        return model::RelationType::Mentor;
    case S::Collaboration: return model::RelationType::Collaboration;
    }
    return model::RelationType::Department;
}

ucf::service::model::IGroupContact::GroupType toServiceGroupType(model::GroupType type)
{
    using S = ucf::service::model::IGroupContact::GroupType;
    switch (type)
    {
    case model::GroupType::Department: return S::Department;
    case model::GroupType::Project:    return S::Project;
    case model::GroupType::Team:       return S::Team;
    case model::GroupType::Custom:     return S::Custom;
    }
    return S::Department;
}

model::GroupType toVMGroupType(ucf::service::model::IGroupContact::GroupType type)
{
    using S = ucf::service::model::IGroupContact::GroupType;
    switch (type)
    {
    case S::Department: return model::GroupType::Department;
    case S::Project:    return model::GroupType::Project;
    case S::Team:       return model::GroupType::Team;
    case S::Custom:     return model::GroupType::Custom;
    }
    return model::GroupType::Department;
}

std::optional<model::GroupType> groupTypeFor(model::RelationType relationType)
{
    switch (relationType)
    {
    case model::RelationType::Department:    return model::GroupType::Department;
    case model::RelationType::Project:       return model::GroupType::Project;
    case model::RelationType::Reporting:     return std::nullopt;
    case model::RelationType::Mentor:        return std::nullopt;
    case model::RelationType::Collaboration: return std::nullopt;
    }
    return std::nullopt;
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
    if (!contactTree || ancestorCandidateId.empty() || startNodeId.empty())
    {
        return false;
    }
    auto currentNode = std::static_pointer_cast<model::IContactTree>(contactTree)->findNodeById(startNodeId);
    while (currentNode)
    {
        if (currentNode->getNodeData().id == ancestorCandidateId)
        {
            return true;
        }
        currentNode = currentNode->getParent().lock();
    }
    return false;
}

VMContactRelation::VMContactRelation(std::string relationId, std::string childId, std::string parentId, RelationType relationType)
    : mRelationId(std::move(relationId))
    , mChildId(std::move(childId))
    , mParentId(std::move(parentId))
    , mRelationType(relationType) {}

std::string                     VMContactRelation::getRelationId()   const { return mRelationId;   }
std::string                     VMContactRelation::getChildId()      const { return mChildId;      }
std::string                     VMContactRelation::getParentId()     const { return mParentId;     }
VMContactRelation::RelationType VMContactRelation::getRelationType() const { return mRelationType; }

} // namespace commonHead::viewModels::utils
