#pragma once

#include <memory>
#include <string>
#include <vector>

#include <ucf/Services/ContactService/IContactEntities.h>
#include <ucf/Services/ContactService/IContactServiceCallback.h>

#include <commonHead/viewModels/ContactListViewModel/IContactListModel.h>

namespace commonHead::viewModels::model {
class ContactTree;
}

namespace commonHead::viewModels::utils {

// ===== Service-entity -> VM-data converters =====
model::ContactNodeData     toVMNodeData (const ucf::service::model::IPersonContactPtr& personContact);
model::ContactNodeData     toVMNodeData (const ucf::service::model::IGroupContactPtr&  groupContact);
model::ContactRelationData toVMRelation (const ucf::service::model::IContactRelationPtr& serviceRelation);

std::vector<model::ContactNodeData>     toVMNodeDatas(const ucf::service::model::PersonContactArray&  personContacts);
std::vector<model::ContactNodeData>     toVMNodeDatas(const ucf::service::model::GroupContactArray&   groupContacts);
std::vector<model::ContactRelationData> toVMRelations(const ucf::service::model::ContactRelationArray& serviceRelations);

model::ContactDirectoryLoadError toVMLoadError(ucf::service::ContactDirectoryLoadError serviceError);

// ===== Tree helpers =====
// Walk up from startNodeId; return true if ancestorCandidateId appears (including startNodeId itself).
bool isAncestorOf(const std::shared_ptr<model::ContactTree>& contactTree,
                  const std::string& ancestorCandidateId,
                  const std::string& startNodeId);

// Minimal ucf::service::IContactRelation impl owned by the VM so we can synthesize
// relation rows to push down to the service via add/updateContactRelations without
// pulling in the service-private ContactEntities.h.
class VMContactRelation final : public ucf::service::model::IContactRelation
{
public:
    VMContactRelation(std::string childId, std::string parentId,
                      RelationType relationType = RelationType::Department);

    std::string  getChildId()      const override;
    std::string  getParentId()     const override;
    RelationType getRelationType() const override;

private:
    std::string  mChildId;
    std::string  mParentId;
    RelationType mRelationType;
};

} // namespace commonHead::viewModels::utils
