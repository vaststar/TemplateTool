#pragma once

#include <source_location>
#include <functional>
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
    virtual ~INetworkHttpManager() = default;
    
    virtual void startHttpNetwork() = 0;
    virtual void sendHttpRestRequest(const ucf::service::network::http::HttpRestRequest& restRequest, const ucf::service::network::http::HttpRestResponseCallbackFunc& restResponseCallback, const std::source_location location = std::source_location::current()) = 0;
    virtual void sendHttpRawRequest(const ucf::service::network::http::HttpRawRequest& rawRequest, const ucf::service::network::http::HttpRawResponseCallbackFunc& rawResponseCallback, const std::source_location location = std::source_location::current()) = 0;
    virtual void downloadContentToMemory(const ucf::service::network::http::HttpDownloadToMemoryRequest& downloadRequest, const ucf::service::network::http::HttpDownloadToMemoryResponseCallbackFunc& downloadResponseCallback, const std::source_location location = std::source_location::current()) = 0;
    virtual void downloadContentToFile(const ucf::service::network::http::HttpDownloadToFileRequest& downloadRequest, const ucf::service::network::http::HttpDownloadToFileResponseCallbackFunc& downloadResponseCallback, const std::source_location location = std::source_location::current()) = 0;
};
}