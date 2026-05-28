#include "CameraDirectoryEntities.h"

#include <utility>

namespace ucf::service::model {

// ===== CameraGroupImpl =====

CameraGroupImpl::CameraGroupImpl(std::string nodeId,
                                 std::string displayName,
                                 CameraNodeStatus status)
    : mNodeId(std::move(nodeId))
    , mDisplayName(std::move(displayName))
    , mStatus(status)
{
}

std::string CameraGroupImpl::getNodeId() const
{
    return mNodeId;
}

std::string CameraGroupImpl::getDisplayName() const
{
    std::scoped_lock lock(mMutex);
    return mDisplayName;
}

CameraNodeStatus CameraGroupImpl::getNodeStatus() const
{
    std::scoped_lock lock(mMutex);
    return mStatus;
}

void CameraGroupImpl::setDisplayName(const std::string& name)
{
    std::scoped_lock lock(mMutex);
    mDisplayName = name;
}

void CameraGroupImpl::setNodeStatus(CameraNodeStatus status)
{
    std::scoped_lock lock(mMutex);
    mStatus = status;
}

// ===== CameraEntryImpl =====

CameraEntryImpl::CameraEntryImpl(std::string nodeId,
                                 std::string displayName,
                                 CameraNodeStatus status,
                                 media::CameraSource source)
    : mNodeId(std::move(nodeId))
    , mDisplayName(std::move(displayName))
    , mStatus(status)
    , mSource(std::move(source))
{
}

std::string CameraEntryImpl::getNodeId() const
{
    return mNodeId;
}

std::string CameraEntryImpl::getDisplayName() const
{
    std::scoped_lock lock(mMutex);
    return mDisplayName;
}

CameraNodeStatus CameraEntryImpl::getNodeStatus() const
{
    std::scoped_lock lock(mMutex);
    return mStatus;
}

media::CameraSource CameraEntryImpl::getSource() const
{
    std::scoped_lock lock(mMutex);
    return mSource;
}

void CameraEntryImpl::setDisplayName(const std::string& name)
{
    std::scoped_lock lock(mMutex);
    mDisplayName = name;
}

void CameraEntryImpl::setNodeStatus(CameraNodeStatus status)
{
    std::scoped_lock lock(mMutex);
    mStatus = status;
}

void CameraEntryImpl::setSource(const media::CameraSource& source)
{
    std::scoped_lock lock(mMutex);
    mSource = source;
}

// ===== CameraDirectoryRelationImpl =====

CameraDirectoryRelationImpl::CameraDirectoryRelationImpl(std::string parentId,
                                                         std::string childId,
                                                         RelationType type)
    : mParentId(std::move(parentId))
    , mChildId(std::move(childId))
    , mRelationType(type)
{
}

std::string CameraDirectoryRelationImpl::getParentId() const
{
    std::scoped_lock lock(mMutex);
    return mParentId;
}

std::string CameraDirectoryRelationImpl::getChildId() const
{
    return mChildId;
}

ICameraDirectoryRelation::RelationType CameraDirectoryRelationImpl::getRelationType() const
{
    std::scoped_lock lock(mMutex);
    return mRelationType;
}

void CameraDirectoryRelationImpl::setParentId(const std::string& parentId)
{
    std::scoped_lock lock(mMutex);
    mParentId = parentId;
}

void CameraDirectoryRelationImpl::setRelationType(RelationType type)
{
    std::scoped_lock lock(mMutex);
    mRelationType = type;
}

} // namespace ucf::service::model
