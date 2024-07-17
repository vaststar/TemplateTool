#pragma once

#include <memory>
#include <ucf/Services/NetworkService/Http/INetworkHttpManager.h>

namespace ucf::utilities::network::http
{
class NetworkHttpRequest;
} // namespace ucf::utilities::network::http

namespace ucf::service::network::http{
class INetworkHttpCallbackHandler;

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
    virtual void sendHttpRestRequest(const ucf::service::network::http::HttpRestRequest& restRequest, const HttpRestResponseCallbackFunc& restResponseCallback, const std::source_location location = std::source_location::current()) override;
private:
    void sendHttpRequest(const ucf::utilities::network::http::NetworkHttpRequest& httpRequest, 
                         std::shared_ptr<INetworkHttpCallbackHandler> callbackHandler,
                         const std::source_location location = std::source_location::current());
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}