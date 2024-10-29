#pragma once

#include <memory>
#include <ucf/Services/NetworkService/Http/INetworkHttpManager.h>

namespace ucf::utilities::network::http
{
class NetworkHttpRequest;
} // namespace ucf::utilities::network::http

namespace ucf::service::network::http{
class INetworkHttpHandler;

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
    virtual void startHttpNetwork() override;
    virtual void sendHttpRestRequest(const ucf::service::network::http::HttpRestRequest& restRequest, const HttpRestResponseCallbackFunc& restResponseCallback, const std::source_location location = std::source_location::current()) override;
    virtual void sendHttpRawRequest(const ucf::service::network::http::HttpRawRequest& rawRequest, const ucf::service::network::http::HttpRawResponseCallbackFunc& rawResponseCallback, const std::source_location location = std::source_location::current()) override;
    virtual void downloadContentToMemory(const ucf::service::network::http::HttpDownloadToMemoryRequest& downloadRequest, const ucf::service::network::http::HttpDownloadToMemoryResponseCallbackFunc& downloadResponseCallback, const std::source_location location = std::source_location::current()) override;
    virtual void downloadContentToFile(const ucf::service::network::http::HttpDownloadToFileRequest& downloadRequest, const ucf::service::network::http::HttpDownloadToFileResponseCallbackFunc& downloadResponseCallback, const std::source_location location = std::source_location::current()) override;
private:
    void sendHttpRequest(std::shared_ptr<INetworkHttpHandler> callbackHandler, const std::source_location location = std::source_location::current());
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}