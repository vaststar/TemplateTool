#pragma once

#include <source_location>
#include <functional>
#include <string>
#include <ucf/Services/ServiceExportMacro/ServiceExport.h>
#include <ucf/Services/NetworkService/Model/HttpDeclareTypes.h>
namespace ucf::service::network::http{
class HttpRestRequest;
class HttpRawRequest;
class HttpDownloadToMemoryRequest;
class HttpDownloadToFileRequest;

class SERVICE_EXPORT INetworkHttpManager
{
public:
    INetworkHttpManager() = default;
    INetworkHttpManager(const INetworkHttpManager&) = delete;
    INetworkHttpManager(INetworkHttpManager&&) = delete;
    INetworkHttpManager& operator=(const INetworkHttpManager&) = delete;
    INetworkHttpManager& operator=(INetworkHttpManager&&) = delete;
    virtual ~INetworkHttpManager() = default;
public:
    virtual void sendHttpRestRequest(const ucf::service::network::http::HttpRestRequest& restRequest, const ucf::service::network::http::HttpRestResponseCallbackFunc& restResponseCallback, const std::source_location location = std::source_location::current()) = 0;
    virtual void sendHttpRawRequest(const ucf::service::network::http::HttpRawRequest& rawRequest, const ucf::service::network::http::HttpRawResponseCallbackFunc& rawResponseCallback, const std::source_location location = std::source_location::current()) = 0;
    virtual void downloadContentToMemory(const ucf::service::network::http::HttpDownloadToMemoryRequest& downloadRequest, const ucf::service::network::http::HttpDownloadToMemoryResponseCallbackFunc& downloadResponseCallback, const std::source_location location = std::source_location::current()) = 0;
    virtual void downloadContentToFile(const ucf::service::network::http::HttpDownloadToFileRequest& downloadRequest, const ucf::service::network::http::HttpDownloadToFileResponseCallbackFunc& downloadResponseCallback, const std::source_location location = std::source_location::current()) = 0;

    /// @brief Cancel an in-flight HTTP request by its requestId.
    ///
    /// Idempotent. If no such request is in flight (already finished, or
    /// unknown id), this is a no-op.
    ///
    /// On success the user's terminal callback fires exactly once with
    /// errorData->errorType == ResponseErrorType::CanceledError. Callers
    /// that initiated the cancel typically detect this and treat it as
    /// the cancel acknowledgement.
    virtual void cancelRequest(const std::string& requestId) = 0;
};
}
