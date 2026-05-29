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

// 摄像机监控视图控制器。
// 只负责目录树 (CameraDirectoryItemModel) + 当前选中节点；
// 视频解码/渲染由各个 MediaCameraView 实例自己持有，controller 不暴露 VideoSink。
class CameraMonitorViewController : public UIViewController
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* cameraTreeModel READ getCameraTreeModel NOTIFY cameraTreeModelChanged)
    Q_PROPERTY(QString selectedCameraId READ getSelectedCameraId NOTIFY selectedCameraChanged)
    Q_PROPERTY(QString selectedCameraName READ getSelectedCameraName NOTIFY selectedCameraChanged)
    QML_ELEMENT
public:
    explicit CameraMonitorViewController(QObject* parent = nullptr);
    ~CameraMonitorViewController() override;

    QAbstractItemModel* getCameraTreeModel() const;
    QString getSelectedCameraId() const;
    QString getSelectedCameraName() const;

public slots:
    // 由 QML tree 在节点单击时调用。仅记录选中状态，并通过 signal 通知 UI；
    // 不直接驱动视频播放——cell 内的 MediaCameraView 自己决定是否拉流。
    Q_INVOKABLE void selectNode(const QString& nodeId);

signals:
    void cameraTreeModelChanged();
    void selectedCameraChanged();

protected:
    void init() override;

private slots:
    // 一对一对应 ICameraDirectoryViewModelCallback；
    // 当前都简单地 rebuildTreeModel()，future 可在此做差量更新。
    void onCameraDirectoryReady();
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
    void rebuildTreeModel();

private:
    std::shared_ptr<commonHead::viewModels::ICameraDirectoryViewModel> mCameraDirectoryViewModel;
    std::shared_ptr<UIVMSignalEmitter::CameraDirectoryViewModelEmitter> mCameraDirectoryEmitter;

    CameraDirectoryItemModel* mCameraTreeModel = nullptr;
    QString mSelectedCameraId;
    QString mSelectedCameraName;
};
