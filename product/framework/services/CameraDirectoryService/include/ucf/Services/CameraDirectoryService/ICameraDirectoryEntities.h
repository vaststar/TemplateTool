#pragma once

#include <memory>
#include <string>
#include <vector>

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>
#include <ucf/Services/MediaService/MediaTypes.h>

namespace ucf::service::model {

enum class CameraNodeStatus {
    Active   = 0,
    Inactive = 1,
    Deleted  = 2,
    Archived = 3
};

// Shared base for camera directory tree nodes (groups and concrete cameras).
class SERVICE_EXPORT ICameraDirectoryNode
{
public:
    virtual ~ICameraDirectoryNode() = default;

    virtual std::string getNodeId() const = 0;
    virtual std::string getDisplayName() const = 0;
    virtual CameraNodeStatus getNodeStatus() const = 0;
};
using ICameraDirectoryNodePtr = std::shared_ptr<ICameraDirectoryNode>;

// Group node (e.g. Local / Remote / Family / Yard).
class SERVICE_EXPORT ICameraGroup : public ICameraDirectoryNode
{
public:
    static std::shared_ptr<ICameraGroup> createInstance(std::string nodeId,
                                                        std::string displayName,
                                                        CameraNodeStatus status = CameraNodeStatus::Active);
};
using ICameraGroupPtr   = std::shared_ptr<ICameraGroup>;
using CameraGroupArray  = std::vector<ICameraGroupPtr>;

// Concrete camera node (local index / network stream).
class SERVICE_EXPORT ICameraEntry : public ICameraDirectoryNode
{
public:
    virtual media::CameraSource getSource() const = 0;

    static std::shared_ptr<ICameraEntry> createInstance(std::string nodeId,
                                                        std::string displayName,
                                                        media::CameraSource source,
                                                        CameraNodeStatus status = CameraNodeStatus::Active);
};
using ICameraEntryPtr   = std::shared_ptr<ICameraEntry>;
using CameraEntryArray  = std::vector<ICameraEntryPtr>;

// Parent-child relation (only Containment is supported today).
class SERVICE_EXPORT ICameraDirectoryRelation
{
public:
    enum class RelationType {
        Containment = 0     // Containment: parent group -> child node (group or camera)
    };

    virtual ~ICameraDirectoryRelation() = default;

    virtual std::string getParentId() const = 0;
    virtual std::string getChildId() const = 0;
    virtual RelationType getRelationType() const = 0;

    static std::shared_ptr<ICameraDirectoryRelation> createInstance(std::string parentId,
                                                                    std::string childId,
                                                                    RelationType type = RelationType::Containment);
};
using ICameraDirectoryRelationPtr   = std::shared_ptr<ICameraDirectoryRelation>;
using CameraDirectoryRelationArray  = std::vector<ICameraDirectoryRelationPtr>;

} // namespace ucf::service::model
