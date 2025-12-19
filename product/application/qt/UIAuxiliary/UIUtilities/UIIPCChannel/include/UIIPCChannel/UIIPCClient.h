#pragma once

#include <memory>
#include <string>
#include <UIUtilities/UIUtilitiesCommonFile/UIUtilitiesExport.h>

namespace UIUtilities{
class UIIPCClient
{

public:
    explicit UIIPCClient(const std::string& serverName, int timeoutMs = 1000);
    ~UIIPCClient();

    UIIPCClient(UIIPCClient&&) = delete;
    UIIPCClient& operator=(UIIPCClient&&) = delete;
    UIIPCClient(const UIIPCClient&) = delete;
    UIIPCClient& operator=(const UIIPCClient&) = delete;

    bool connect();

    bool isConnected() const;

    bool send(const std::string& msg);

    void close();
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};
}
