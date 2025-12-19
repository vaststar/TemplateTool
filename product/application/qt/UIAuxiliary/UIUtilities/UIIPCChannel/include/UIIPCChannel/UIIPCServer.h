#pragma once

#include <string>
#include <functional>
#include <UIUtilities/UIUtilitiesCommonFile/UIUtilitiesExport.h>

namespace UIUtilities{

class UIIPCServer final
{
public:
    using MessageHandler = std::function<void(const std::string&)>;

    explicit UIIPCServer(std::string serverName);
    ~UIIPCServer();

    UIIPCServer(UIIPCServer&&) = delete;
    UIIPCServer& operator=(UIIPCServer&&) = delete;

    UIIPCServer(const UIIPCServer&) = delete;
    UIIPCServer& operator=(const UIIPCServer&) = delete;

    // 启动监听（主实例调用）
    bool start();

    // 设置收到消息的回调（主实例调用）
    void setMessageHandler(MessageHandler handler);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};
}
