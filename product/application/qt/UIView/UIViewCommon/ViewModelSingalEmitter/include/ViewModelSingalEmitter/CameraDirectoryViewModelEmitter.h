#pragma once

#include "RegisterViewModelMetaTypes.h"

#include <QObject>
#include <QString>
#include <string>
#include <vector>

#include <commonHead/viewModels/CameraDirectoryViewModel/ICameraDirectoryViewModel.h>

namespace UIVMSignalEmitter {

// 一一对应 ICameraDirectoryViewModelCallback 的事件，透传 payload，
// 让消费方可以按需做差量 UI 更新。
class CameraDirectoryViewModelEmitter
    : public QObject
    , public commonHead::viewModels::ICameraDirectoryViewModelCallback
{
    Q_OBJECT
public:
    using NodeData = commonHead::viewModels::model::CameraDirectoryNodeData;
    using RelationData = commonHead::viewModels::model::CameraDirectoryRelationData;

    explicit CameraDirectoryViewModelEmitter(QObject* parent = nullptr)
        : QObject(parent) {}

    void onCameraDirectoryReady() override
    { emit signals_onCameraDirectoryReady(); }

    void onCameraDirectoryLoadFailed(commonHead::viewModels::model::CameraDirectoryLoadError error) override
    { emit signals_onCameraDirectoryLoadFailed(error); }

    void onCameraGroupsAdded(const std::vector<NodeData>& groups) override
    { emit signals_onCameraGroupsAdded(groups); }
    void onCameraGroupsUpdated(const std::vector<NodeData>& groups) override
    { emit signals_onCameraGroupsUpdated(groups); }
    void onCameraGroupsRemoved(const std::vector<std::string>& ids) override
    { emit signals_onCameraGroupsRemoved(ids); }

    void onCamerasAdded(const std::vector<NodeData>& cameras) override
    { emit signals_onCamerasAdded(cameras); }
    void onCamerasUpdated(const std::vector<NodeData>& cameras) override
    { emit signals_onCamerasUpdated(cameras); }
    void onCamerasRemoved(const std::vector<std::string>& ids) override
    { emit signals_onCamerasRemoved(ids); }

    void onCameraRelationsAdded(const std::vector<RelationData>& relations) override
    { emit signals_onCameraRelationsAdded(relations); }
    void onCameraRelationsUpdated(const std::vector<RelationData>& relations) override
    { emit signals_onCameraRelationsUpdated(relations); }
    void onCameraRelationsRemoved(const std::vector<std::string>& ids) override
    { emit signals_onCameraRelationsRemoved(ids); }

    void onCurrentCameraChanged(const std::string& nodeId) override
    { emit signals_onCurrentCameraChanged(QString::fromStdString(nodeId)); }

signals:
    void signals_onCameraDirectoryReady();
    void signals_onCameraDirectoryLoadFailed(commonHead::viewModels::model::CameraDirectoryLoadError error);

    void signals_onCameraGroupsAdded(const std::vector<NodeData>& groups);
    void signals_onCameraGroupsUpdated(const std::vector<NodeData>& groups);
    void signals_onCameraGroupsRemoved(const std::vector<std::string>& ids);

    void signals_onCamerasAdded(const std::vector<NodeData>& cameras);
    void signals_onCamerasUpdated(const std::vector<NodeData>& cameras);
    void signals_onCamerasRemoved(const std::vector<std::string>& ids);

    void signals_onCameraRelationsAdded(const std::vector<RelationData>& relations);
    void signals_onCameraRelationsUpdated(const std::vector<RelationData>& relations);
    void signals_onCameraRelationsRemoved(const std::vector<std::string>& ids);

    void signals_onCurrentCameraChanged(const QString& nodeId);
};

} // namespace UIVMSignalEmitter
