#pragma once

#include <source_location>
#include <functional>
#include <ucf/Services/ServiceCommonFile/ServiceExport.h>
#include <ucf/Services/NetworkService/Model/HttpDeclareTypes.h>
namespace ucf::service::network::http{
class HttpRestRequest;
class HttpRawRequest;
class HttpDownloadToContentRequest;

class SERVICE_EXPORT INetworkHttpManager
{
public:
    virtual ~INetworkHttpManager() = default;
    
    virtual void sendHttpRestRequest(const ucf::service::network::http::HttpRestRequest& restRequest, const ucf::service::network::http::HttpRestResponseCallbackFunc& restResponseCallback, const std::source_location location = std::source_location::current()) = 0;
    virtual void sendHttpRawRequest(const ucf::service::network::http::HttpRawRequest& rawRequest, const ucf::service::network::http::HttpRawResponseCallbackFunc& rawResponseCallback, const std::source_location location = std::source_location::current()) = 0;
    virtual void downloadContentToMemory(const ucf::service::network::http::HttpDownloadToContentRequest& downloadRequest, const ucf::service::network::http::HttpDownloadToContentResponseCallbackFunc& downloadResponseCallback, const std::source_location location = std::source_location::current()) = 0;
};
}