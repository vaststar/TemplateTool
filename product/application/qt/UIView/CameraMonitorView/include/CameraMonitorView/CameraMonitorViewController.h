#pragma once

#include <memory>

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QtQml>

#include "UIViewBase/UIViewController.h"
#include "CameraDirectoryItemModel.h"
// Must precede moc-generated slot signatures so the Q_DECLARE_METATYPE
// specializations win over the primary template (avoids C2908).
#include "ViewModelSingalEmitter/RegisterViewModelMetaTypes.h"

namespace commonHead::viewModels {
    class ICameraDirectoryViewModel;
}

namespace UIVMSignalEmitter {
    class CameraDirectoryViewModelEmitter;
}

// Thin controller: VM callbacks are forwarded to CameraDirectoryItemModel,
// which owns the begin/end model-change signaling.
class CameraMonitorViewController : public UIViewController
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* cameraTreeModel READ getCameraTreeModel NOTIFY cameraTreeModelChanged)
    Q_PROPERTY(QString currentCameraId READ getCurrentCameraId NOTIFY currentCameraChanged)
    Q_PROPERTY(QString currentCameraName READ getCurrentCameraName NOTIFY currentCameraChanged)
    Q_PROPERTY(LoadState loadState READ getLoadState NOTIFY loadStateChanged)
    QML_ELEMENT
public:
    enum LoadState { Loading = 0, Ready = 1, Error = 2 };
    Q_ENUM(LoadState)

    explicit CameraMonitorViewController(QObject* parent = nullptr);
    ~CameraMonitorViewController() override;

    QAbstractItemModel* getCameraTreeModel() const;
    QString getCurrentCameraId() const;
    QString getCurrentCameraName() const;
    LoadState getLoadState() const;

public slots:
    Q_INVOKABLE void selectNode(const QString& nodeId);

    // Drag-drop / re-parent.
    Q_INVOKABLE bool canDropOnNode(const QString& srcId, const QString& targetParentId) const;
    Q_INVOKABLE void moveCameraNode(const QString& srcId, const QString& targetParentId);

    // Preview-panel helpers (consumed by CameraPreviewPanel.qml).
    // Returns {} for unknown / non-camera nodes; otherwise:
    //   { "kind": "local",   "index": int }
    //   { "kind": "network", "url": str, "transport": str,
    //                        "openTimeoutMs": int, "readTimeoutMs": int }
    Q_INVOKABLE QVariantMap cameraSourceForNode(const QString& nodeId) const;

    // Pops a standalone preview window for nodeId. No-op for unknown ids.
    Q_INVOKABLE void openCameraWindow(const QString& nodeId);

signals:
    void cameraTreeModelChanged();
    void currentCameraChanged();
    void loadStateChanged();
    void nodeMoved(QString newParentId);

protected:
    void init() override;

private slots:
    void onCameraDirectoryReady();
    void onCameraDirectoryLoadFailed(commonHead::viewModels::model::CameraDirectoryLoadError error);
    void onCurrentCameraChanged(const QString& nodeId);
    void onCameraGroupsAdded(const std::vector<commonHead::viewModels::model::CameraDirectoryNodeData>& groups);
    void onCameraGroupsUpdated(const std::vector<commonHead::viewModels::model::CameraDirectoryNodeData>& groups);
    void onCameraGroupsRemoved(const std::vector<std::string>& ids);
    void onCamerasAdded(const std::vector<commonHead::viewModels::model::CameraDirectoryNodeData>& cameras);
    void onCamerasUpdated(const std::vector<commonHead::viewModels::model::CameraDirectoryNodeData>& cameras);
    void onCamerasRemoved(const std::vector<std::string>& ids);
    void onCameraRelationsAdded(const std::vector<commonHead::viewModels::model::CameraDirectoryRelationData>& relations);
    void onCameraRelationsUpdated(const std::vector<commonHead::viewModels::model::CameraDirectoryRelationData>& relations);
    void onCameraRelationsRemoved(const std::vector<std::string>& ids);

private:
    void setLoadState(LoadState s);

private:
    std::shared_ptr<commonHead::viewModels::ICameraDirectoryViewModel> mCameraDirectoryViewModel;
    std::shared_ptr<UIVMSignalEmitter::CameraDirectoryViewModelEmitter> mCameraDirectoryEmitter;

    CameraDirectoryItemModel* mCameraTreeModel = nullptr;
    QString   mCurrentCameraId;
    QString   mCurrentCameraName;
    LoadState mLoadState = Loading;
    bool      mHasPendingMove = false;
    QString   mPendingMoveParent;
};
