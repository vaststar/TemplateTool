#pragma once

#include <map>
#include <memory>
#include <curl/curl.h>
#include <ucf/Services/NetworkService/NetworkModelTypes/Http/NetworkHttpTypes.h>
namespace ucf::network::libcurl{
class LibCurlEasyHandle
{
public:
    LibCurlEasyHandle(const ucf::network::http::HttpHeaderCallback& headerCallback, const ucf::network::http::HttpBodyCallback& bodyCallback, const ucf::network::http::HttpCompletionCallback& completionCallback);
    ~LibCurlEasyHandle();
    LibCurlEasyHandle(const LibCurlEasyHandle&) = delete;
    LibCurlEasyHandle(LibCurlEasyHandle&&) = delete;
    LibCurlEasyHandle& operator=(const LibCurlEasyHandle&) = delete;
    LibCurlEasyHandle& operator=(LibCurlEasyHandle&&) = delete;
public:
    void setHttpMethod(ucf::network::http::HTTPMethod method);
    void setURI(const std::string& uri);
    void setHeaders(const ucf::network::http::NetworkHttpHeaders& headers);
    void setTrackingId(const std::string& trackingId);
    void setTimeout(int timeoutSecs);
    void setCommonOptions();

    void finishHandle(CURLcode code);

    void addResponseHeader(const std::string& key, const std::string& val);
    void headersCompleted();

    void appendResponseBody(char *data, size_t size);
public:
    CURL* getHandle() const;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}