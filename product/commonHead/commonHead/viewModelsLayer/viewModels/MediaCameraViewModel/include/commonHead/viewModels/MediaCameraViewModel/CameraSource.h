#pragma once

#include <string>
#include <variant>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

namespace commonHead::viewModels::model {

// VM 层"本地摄像头"描述（与 Service 层 ucf::service::media::LocalCameraSource 同构，
// 但有意分离以保持 UI ↔ ViewModel ↔ Service 的分层）
struct COMMONHEAD_EXPORT LocalCameraSource
{
    int index = 0;
};

// VM 层"网络流摄像头"描述
struct COMMONHEAD_EXPORT NetworkCameraSource
{
    std::string url;
    std::string transport;     // "tcp" / "udp"，留空使用默认
    int openTimeoutMs = 5000;
    int readTimeoutMs = 5000;
};

// 封闭集合多态：后续可扩展 FileSource、VirtualSource 等
using CameraSource = std::variant<LocalCameraSource, NetworkCameraSource>;

// 简单展示串，用于日志 / UI debug
COMMONHEAD_EXPORT std::string toDisplayString(const CameraSource& source);

}
