#pragma once

#include <memory>
#include <mutex>
#include <atomic>
#include <string>
#include <unordered_map>
#include <vector>

#include "CameraDirectoryEntities.h"
#include "CameraDirectoryNotificationSink.h"

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

class CameraDirectoryDBAccess;

// Coordinates the in-memory view with database persistence:
//   - In-memory layer: batched CRUD protected by a mutex.
//   - Persistence layer: owns CameraDirectoryDBAccess and flushes accepted writes to disk.
class CameraDirectoryModel
{
public:
    explicit CameraDirectoryModel(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~CameraDirectoryModel();
    CameraDirectoryModel(const CameraDirectoryModel&) = delete;
    CameraDirectoryModel(CameraDirectoryModel&&) = delete;
    CameraDirectoryModel& operator=(const CameraDirectoryModel&) = delete;
    CameraDirectoryModel& operator=(CameraDirectoryModel&&) = delete;

public:
    // ===== Read =====
    model::CameraGroupArray             getCameraGroups() const;
    model::CameraEntryArray             getCameras() const;
    model::CameraDirectoryRelationArray getCameraRelations() const;
    model::ICameraGroupPtr              getCameraGroup(const std::string& nodeId) const;
    model::ICameraEntryPtr              getCamera(const std::string& nodeId) const;

    // ===== Batch write: memory first then DB; returns items that actually took effect =====
    model::CameraGroupArray addCameraGroups(const model::CameraGroupArray& groups);
    model::CameraGroupArray updateCameraGroups(const model::CameraGroupArray& groups);
    std::vector<std::string> removeCameraGroups(const std::vector<std::string>& nodeIds);

    model::CameraEntryArray addCameras(const model::CameraEntryArray& cameras);
    model::CameraEntryArray updateCameras(const model::CameraEntryArray& cameras);
    std::vector<std::string> removeCameras(const std::vector<std::string>& nodeIds);

    model::CameraDirectoryRelationArray addCameraRelations(const model::CameraDirectoryRelationArray& relations);
    model::CameraDirectoryRelationArray updateCameraRelations(const model::CameraDirectoryRelationArray& relations);
    std::vector<std::string>            removeCameraRelations(const std::vector<std::string>& relationIds);

    // ===== Lifecycle =====
    // Bind the database id only; does not start any load. Idempotent.
    void bindDatabase(const std::string& databaseId);

    // Load the three data classes from the database and populate memory; requires bindDatabase first.
    // See ICameraDirectoryService::loadCameraDirectory for full semantics.
    void loadCameraDirectory();

    // True when the in-memory view is fully populated from the database.
    bool isCameraDirectoryReady() const;

    // ===== Notification sink =====
    // Injected by Service; weak_ptr so async DB callbacks arriving after sink destruction
    // don't dangle.
    void setNotificationSink(std::weak_ptr<ICameraDirectoryNotificationSink> sink);

private:
    // Memory-only primitives (do not touch the DB).
    model::CameraGroupArray             addCameraGroupsInMemory(const model::CameraGroupArray& groups);
    model::CameraGroupArray             updateCameraGroupsInMemory(const model::CameraGroupArray& groups);
    std::vector<std::string>            removeCameraGroupsInMemory(const std::vector<std::string>& nodeIds);
    model::CameraEntryArray             addCamerasInMemory(const model::CameraEntryArray& cameras);
    model::CameraEntryArray             updateCamerasInMemory(const model::CameraEntryArray& cameras);
    std::vector<std::string>            removeCamerasInMemory(const std::vector<std::string>& nodeIds);
    model::CameraDirectoryRelationArray addCameraRelationsInMemory(const model::CameraDirectoryRelationArray& relations);
    model::CameraDirectoryRelationArray updateCameraRelationsInMemory(const model::CameraDirectoryRelationArray& relations);
    std::vector<std::string>            removeCameraRelationsInMemory(const std::vector<std::string>& relationIds);

    // Single exit points that emit the load-finished / load-failed notification
    // (thread-safe; fires exactly once per load).
    void finishLoadSuccess();
    void finishLoadFailure(CameraDirectoryLoadError error);

    enum class LoadStage : std::uint8_t {
        Uninit,    // bindDatabase has not been called.
        DbBound,   // bindDatabase done; load not started.
        Loading,   // At least one chunk load is in flight.
        Ready,     // All chunks succeeded.
        Failed,    // Load failed (in-memory data preserved; retryable).
    };

private:
    mutable std::mutex mMutex;
    std::unordered_map<std::string, std::shared_ptr<model::CameraGroupImpl>>             mGroups;
    std::unordered_map<std::string, std::shared_ptr<model::CameraEntryImpl>>             mCameras;
    // Keyed by the relation's surrogate primary key (RELATION_ID, UUID).
    std::unordered_map<std::string, std::shared_ptr<model::CameraDirectoryRelationImpl>> mRelations;

    const std::unique_ptr<CameraDirectoryDBAccess> mCameraDirectoryDBAccess;

    // Sink is set once and then read-only; no locking needed. Write paths lock() it once per call.
    std::weak_ptr<ICameraDirectoryNotificationSink> mNotificationSink;

    // Load-stage state machine; atomic so chunk callbacks and the calling thread see consistent values.
    std::atomic<LoadStage> mLoadStage{LoadStage::Uninit};

    // Set when loadCameraDirectory() is called before bindDatabase(); bindDatabase()
    // then auto-promotes the load once the DB id has been set. Lets callers be order
    // independent: registering and calling load before the DB is ready is safe.
    std::atomic<bool> mLoadPending{false};
};

} // namespace ucf::service
