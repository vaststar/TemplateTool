#pragma once

#include <mutex>
#include <string>

#include <ucf/Services/CameraDirectoryService/ICameraDirectoryEntities.h>

namespace ucf::service::model {

// Concrete in-memory implementations; keep mutable state out of the public interfaces.
class CameraGroupImpl : public ICameraGroup
{
public:
    CameraGroupImpl(std::string nodeId,
                    std::string displayName,
                    CameraNodeStatus status);

    std::string getNodeId() const override;
    std::string getDisplayName() const override;
    CameraNodeStatus getNodeStatus() const override;

    void setDisplayName(const std::string& name);
    void setNodeStatus(CameraNodeStatus status);

private:
    mutable std::mutex mMutex;
    const std::string mNodeId;
    std::string mDisplayName;
    CameraNodeStatus mStatus{CameraNodeStatus::Active};
};

class CameraEntryImpl : public ICameraEntry
{
public:
    CameraEntryImpl(std::string nodeId,
                    std::string displayName,
                    CameraNodeStatus status,
                    media::CameraSource source);

    std::string getNodeId() const override;
    std::string getDisplayName() const override;
    CameraNodeStatus getNodeStatus() const override;
    media::CameraSource getSource() const override;

    void setDisplayName(const std::string& name);
    void setNodeStatus(CameraNodeStatus status);
    void setSource(const media::CameraSource& source);

private:
    mutable std::mutex mMutex;
    const std::string mNodeId;
    std::string mDisplayName;
    CameraNodeStatus mStatus{CameraNodeStatus::Active};
    media::CameraSource mSource;
};

class CameraDirectoryRelationImpl : public ICameraDirectoryRelation
{
public:
    CameraDirectoryRelationImpl(std::string parentId,
                                std::string childId,
                                RelationType type);

    std::string getParentId() const override;
    std::string getChildId() const override;
    RelationType getRelationType() const override;

    void setParentId(const std::string& parentId);
    void setRelationType(RelationType type);

private:
    mutable std::mutex mMutex;
    std::string mParentId;
    const std::string mChildId;
    RelationType mRelationType;
};

} // namespace ucf::service::model
