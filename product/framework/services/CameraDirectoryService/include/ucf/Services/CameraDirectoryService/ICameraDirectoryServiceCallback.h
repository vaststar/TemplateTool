#pragma once

#include <string>
#include <vector>

#include <ucf/Services/CameraDirectoryService/ICameraDirectoryEntities.h>

namespace ucf::service {

// Why a directory load may fail; loadCameraDirectory() can be retried after a failure.
enum class CameraDirectoryLoadError
{
    Unknown,            // Catch-all when no more specific cause is known.
    DatabaseNotBound,   // loadCameraDirectory was invoked before the database was bound.
    DatabaseReadFailed, // The underlying database read returned an error.
};

class ICameraDirectoryServiceCallback
{
public:
    ICameraDirectoryServiceCallback() = default;
    ICameraDirectoryServiceCallback(const ICameraDirectoryServiceCallback&) = delete;
    ICameraDirectoryServiceCallback(ICameraDirectoryServiceCallback&&) = delete;
    ICameraDirectoryServiceCallback& operator=(const ICameraDirectoryServiceCallback&) = delete;
    ICameraDirectoryServiceCallback& operator=(ICameraDirectoryServiceCallback&&) = delete;
    virtual ~ICameraDirectoryServiceCallback() = default;

public:
    // Directory load finished (fired after loadCameraDirectory() has populated all data).
    virtual void onCameraDirectoryReady() {}

    // Directory load failed; the model stays in the bound state and loadCameraDirectory()
    // can be invoked again to retry.
    virtual void onCameraDirectoryLoadFailed(CameraDirectoryLoadError /*error*/) {}

    // Group events (batched; a single-item change is still delivered as an array of size 1).
    virtual void onCameraGroupsAdded(const model::CameraGroupArray& /*groups*/) {}
    virtual void onCameraGroupsUpdated(const model::CameraGroupArray& /*groups*/) {}
    virtual void onCameraGroupsRemoved(const std::vector<std::string>& /*nodeIds*/) {}

    // Camera events.
    virtual void onCamerasAdded(const model::CameraEntryArray& /*cameras*/) {}
    virtual void onCamerasUpdated(const model::CameraEntryArray& /*cameras*/) {}
    virtual void onCamerasRemoved(const std::vector<std::string>& /*nodeIds*/) {}

    // Relation events.
    virtual void onCameraRelationsAdded(const model::CameraDirectoryRelationArray& /*relations*/) {}
    virtual void onCameraRelationsUpdated(const model::CameraDirectoryRelationArray& /*relations*/) {}
    virtual void onCameraRelationsRemoved(const std::vector<std::string>& /*childIds*/) {}
};

} // namespace ucf::service
