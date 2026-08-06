#pragma once

#include <string>
#include <vector>

#include <ucf/Services/CameraDirectoryService/ICameraDirectoryServiceCallback.h>
#include "CameraDirectoryEntities.h"

namespace ucf::service {

// Change origin; lets upstream decide UI / persistence / replication policy per source.
enum class CameraDirectoryNotificationSource {
    Local,      // Triggered by an explicit Service API call from above.
    Remote,     // Remote / server push (reserved).
    DbLoad,     // Startup load / rebuild (reserved).
    Reconcile,  // Background reconciliation (reserved).
};

// Internal "notification sink": Model invokes the sink right after a state change is produced;
// Service implements the sink and translates each call into an outward
// ICameraDirectoryServiceCallback notification.
//
// Discipline (must be followed to avoid duplicate / missing events):
//   1. The private *InMemory primitives never call the sink.
//   2. Only the function that "produces a new fact" calls the sink; today that is restricted
//      to CameraDirectoryModel's public write methods.
//   3. Manager is a transparent forwarder by default and does not call the sink.
//   4. Sink implementers must not re-enter Model write APIs from within a callback.
//   5. During load (loadCameraDirectory) only *InMemory primitives are used to backfill;
//      no per-item add events are emitted. The load lifecycle is summarized via a single
//      onDirectoryLoaded / onDirectoryLoadFailed at the end.
class ICameraDirectoryNotificationSink
{
public:
    virtual ~ICameraDirectoryNotificationSink() = default;

    virtual void onGroupsAdded   (const model::CameraGroupArray& groups,    CameraDirectoryNotificationSource src) = 0;
    virtual void onGroupsUpdated (const model::CameraGroupArray& groups,    CameraDirectoryNotificationSource src) = 0;
    virtual void onGroupsRemoved (const std::vector<std::string>& nodeIds,  CameraDirectoryNotificationSource src) = 0;

    virtual void onCamerasAdded  (const model::CameraEntryArray& cameras,   CameraDirectoryNotificationSource src) = 0;
    virtual void onCamerasUpdated(const model::CameraEntryArray& cameras,   CameraDirectoryNotificationSource src) = 0;
    virtual void onCamerasRemoved(const std::vector<std::string>& nodeIds,  CameraDirectoryNotificationSource src) = 0;

    virtual void onRelationsAdded  (const model::CameraDirectoryRelationArray& relations, CameraDirectoryNotificationSource src) = 0;
    virtual void onRelationsUpdated(const model::CameraDirectoryRelationArray& relations, CameraDirectoryNotificationSource src) = 0;
    virtual void onRelationsRemoved(const std::vector<std::string>& relationIds,             CameraDirectoryNotificationSource src) = 0;

    // Load-finished notification (fires exactly once after loadCameraDirectory has populated
    // all three categories).
    virtual void onDirectoryLoaded() {}

    // Load-failed notification; the error code is supplied by the caller and is suitable
    // for logging / UI error display.
    virtual void onDirectoryLoadFailed(CameraDirectoryLoadError /*error*/) {}
};

} // namespace ucf::service
