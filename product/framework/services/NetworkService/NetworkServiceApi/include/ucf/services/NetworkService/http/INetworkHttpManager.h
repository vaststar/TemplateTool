#pragma once

#include <source_location>
#include <functional>
#include <string>
#include <ucf/services/NetworkService/model/HttpDeclareTypes.h>
namespace ucf::service::network::http{
class HttpRestRequest;
class HttpRawRequest;
class HttpDownloadToMemoryRequest;
class HttpDownloadToFileRequest;

/// @brief HTTP requests with callback-based responses.
///
/// Callbacks have no fixed thread affinity. Normal network responses run on the
/// network worker; submission failures and cancellation can invoke callbacks on
/// the submitting or canceling thread, including before the API call returns.
/// Callbacks must not throw. Callers must synchronize their own state and avoid
/// holding locks that their callbacks may acquire while submitting or canceling
/// a request.
///
/// REST/raw callbacks report a terminal response. Download callbacks can report
/// multiple progress responses (isFinished() == false), followed by a terminal
/// response (isFinished() == true). A terminal response does not by itself mean
/// HTTP success; inspect its error data and HTTP status code.
///
/// Response objects and references returned by their accessors are borrowed only
/// for the current callback invocation. Copy needed values before storing them
/// or dispatching work to another thread. The manager copies the request data
/// and callback during submission; the caller need not keep the request object
/// alive after the submission call returns.
class INetworkHttpManager
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
    /// A terminal response with ResponseErrorType::CanceledError acknowledges
    /// cancellation. Cancellation races with normal completion, so a normal
    /// terminal result may still be delivered if completion already claimed it.
    ///
    /// This call can invoke the terminal callback before returning, or defer it
    /// until pending registration or an active network callback has finished.
    /// Returning is not a callback-drain barrier and does not guarantee that no
    /// callback is running or will run. Callers must protect callback-owned
    /// state during teardown independently of cancellation.
    virtual void cancelRequest(const std::string& requestId) = 0;
};
}
