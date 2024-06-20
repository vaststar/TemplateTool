#pragma once

#include <memory>
#include <curl/curl.h>
#include <ucf/Services/NetworkService/NetworkModelTypes/Http/NetworkHttpTypes.h>
namespace ucf::network::libcurl{
class LibCurlEasyHandle
{
public:
    LibCurlEasyHandle();
    ~LibCurlEasyHandle();
    LibCurlEasyHandle(const LibCurlEasyHandle&) = delete;
    LibCurlEasyHandle(LibCurlEasyHandle&&) = delete;
    LibCurlEasyHandle& operator=(const LibCurlEasyHandle&) = delete;
    LibCurlEasyHandle& operator=(LibCurlEasyHandle&&) = delete;
public:
    void setHttpMethod();
    void setCallbacks(const ucf::network::http::HttpHeaderCallback& headerCallback, const ucf::network::http::HttpBodyCallback& bodyCallback, const ucf::network::http::HttpCompletionCallback& completionCallback);
    void setTrackingId(const std::string& trackingId);

    void finishHandle(CURLcode code);
public:
    CURL* getHandle() const;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}