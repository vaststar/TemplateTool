#pragma once

#include <string>
#include <vector>

#include <ucf/Services/CameraDirectoryService/ICameraDirectoryEntities.h>

namespace ucf::service {

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
    // 目录加载完成（数据库初始化后触发）
    virtual void onCameraDirectoryReady() {}

    // 分组事件（按数组下发，单条变更亦传 size==1 的数组）
    virtual void onCameraGroupsAdded(const model::CameraGroupArray& /*groups*/) {}
    virtual void onCameraGroupsUpdated(const model::CameraGroupArray& /*groups*/) {}
    virtual void onCameraGroupsRemoved(const std::vector<std::string>& /*nodeIds*/) {}

    // 摄像头事件
    virtual void onCamerasAdded(const model::CameraEntryArray& /*cameras*/) {}
    virtual void onCamerasUpdated(const model::CameraEntryArray& /*cameras*/) {}
    virtual void onCamerasRemoved(const std::vector<std::string>& /*nodeIds*/) {}

    // 关系事件
    virtual void onCameraRelationsAdded(const model::CameraDirectoryRelationArray& /*relations*/) {}
    virtual void onCameraRelationsUpdated(const model::CameraDirectoryRelationArray& /*relations*/) {}
    virtual void onCameraRelationsRemoved(const std::vector<std::string>& /*childIds*/) {}
};

} // namespace ucf::service
