#pragma once

#include <memory>
#include <ucf/Services/NetworkService/Http/INetworkHttpManager.h>
namespace ucf::network::http{
class NetworkHttpManager final: public INetworkHttpManager,
                                public std::enable_shared_from_this<NetworkHttpManager>
{
public:
    NetworkHttpManager();
    virtual ~NetworkHttpManager();
    NetworkHttpManager(const NetworkHttpManager&) = delete;
    NetworkHttpManager(NetworkHttpManager&&) = delete;
    NetworkHttpManager& operator=(const NetworkHttpManager&) = delete;
    NetworkHttpManager& operator=(NetworkHttpManager&&) = delete;
public:
    virtual void sendHttpRequest(const NetworkHttpRequest& httpRequest, const NetworkHttpResponseCallbackFunc& callBackFunc) override;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}