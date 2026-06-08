#pragma once

#include <memory>

#include <QObject>
#include <QString>
#include <QtQml>

#include "UIViewBase/UIViewController.h"
#include "CameraDirectoryItemModel.h"
// Pull in Q_DECLARE_METATYPE specializations BEFORE moc generates code for
// our slot signatures — otherwise QMetaTypeId<T> gets primary-instantiated
// and the explicit specialization later fails with C2908.
#include "ViewModelSingalEmitter/RegisterViewModelMetaTypes.h"

namespace commonHead::viewModels {
    class ICameraDirectoryViewModel;
}

namespace UIVMSignalEmitter {
    class CameraDirectoryViewModelEmitter;
}

// Thin controller: 9 个回调槽都只把 VM payload 转给 CameraDirectoryItemModel
// 的对应 mutation 方法，让 model 自己发 begin/end insert/update/remove/move 信号。
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

    // ===== Drag-drop / re-parent =====
    // Pre-flight: returns true when dragging `srcId` onto `targetParentId` would
    // produce a valid move. Empty `targetParentId` means "drop on root".
    Q_INVOKABLE bool canDropOnNode(const QString& srcId, const QString& targetParentId) const;
    // Apply the move. Forwards to the VM; the change becomes visible through the
    // usual relation-callback round-trip.
    Q_INVOKABLE void moveCameraNode(const QString& srcId, const QString& targetParentId);

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
