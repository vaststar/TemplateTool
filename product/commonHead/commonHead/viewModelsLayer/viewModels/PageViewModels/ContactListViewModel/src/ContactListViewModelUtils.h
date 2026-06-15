#pragma once

#include <memory>
#include <optional>
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

// ===== Enum bridges between VM and service layers =====
ucf::service::model::IContactRelation::RelationType toServiceRelationType(model::RelationType type);
model::RelationType                                  toVMRelationType(ucf::service::model::IContactRelation::RelationType type);
ucf::service::model::IGroupContact::GroupType        toServiceGroupType(model::GroupType type);
model::GroupType                                     toVMGroupType(ucf::service::model::IGroupContact::GroupType type);

// Maps a relation slice to the kind of group it expects as the parent node, when one
// exists. RelationTypes whose parent is always a Person (Reporting / Mentor) or whose
// parent shape is undefined here (Collaboration is peer-to-peer) return std::nullopt;
// callers should treat that as "there are no group nodes to load for this slice".
std::optional<model::GroupType> groupTypeFor(model::RelationType relationType);

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
    // relationId may be empty when issuing an "add" request; the service mints a new
    // UUID in that case. For update requests callers must pass the existing id.
    VMContactRelation(std::string relationId,
                      std::string childId,
                      std::string parentId,
                      RelationType relationType = RelationType::Folder);

    std::string  getRelationId()   const override;
    std::string  getChildId()      const override;
    std::string  getParentId()     const override;
    RelationType getRelationType() const override;

private:
    std::string  mRelationId;
    std::string  mChildId;
    std::string  mParentId;
    RelationType mRelationType;
};

// Minimal ucf::service::IPersonContact impl owned by the VM so add/updateContact can
// synthesize person rows to push down to the service without pulling in the
// service-private ContactEntities.h. Only the fields the service write path consumes
// (id / name / status) are carried; profile sub-fields are reported empty.
class VMPersonContact final : public ucf::service::model::IPersonContact
{
public:
    VMPersonContact(std::string contactId, std::string personName,
                    ContactStatus status = ContactStatus::Active);

    std::string   getContactId()     const override;
    ContactStatus getContactStatus() const override;
    std::string   getPersonName()    const override;
    std::string   getFirstName()     const override;
    std::string   getLastName()      const override;
    Gender        getGender()        const override;
    std::string   getPhone()         const override;
    std::string   getEmail()         const override;

private:
    std::string   mContactId;
    std::string   mPersonName;
    ContactStatus mStatus;
};

// Minimal ucf::service::IGroupContact impl owned by the VM. The service add path inspects
// getGroupType() to build the matching concrete sub-class; typed sub-fields stay default.
class VMGroupContact final : public ucf::service::model::IGroupContact
{
public:
    VMGroupContact(std::string contactId, std::string groupName, GroupType groupType,
                   ContactStatus status = ContactStatus::Active);

    std::string   getContactId()     const override;
    ContactStatus getContactStatus() const override;
    std::string   getGroupName()     const override;
    GroupType     getGroupType()     const override;

private:
    std::string   mContactId;
    std::string   mGroupName;
    GroupType     mGroupType;
    ContactStatus mStatus;
};

} // namespace commonHead::viewModels::utils
