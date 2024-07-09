#pragma once

#include <map>
#include <memory>
#include <curl/curl.h>
#include <ucf/Services/NetworkService/NetworkModelTypes/Http/NetworkHttpTypes.h>
namespace ucf::service::network::libcurl{
class LibCurlEasyHandle
{
public:
    LibCurlEasyHandle(const ucf::service::network::http::HttpHeaderCallback& headerCallback, const ucf::service::network::http::HttpBodyCallback& bodyCallback, const ucf::service::network::http::HttpCompletionCallback& completionCallback);
    ~LibCurlEasyHandle();
    LibCurlEasyHandle(const LibCurlEasyHandle&) = delete;
    LibCurlEasyHandle(LibCurlEasyHandle&&) = delete;
    LibCurlEasyHandle& operator=(const LibCurlEasyHandle&) = delete;
    LibCurlEasyHandle& operator=(LibCurlEasyHandle&&) = delete;
public:
    void setHttpMethod(ucf::service::network::http::HTTPMethod method);
    void setURI(const std::string& uri);
    void setHeaders(const ucf::service::network::http::NetworkHttpHeaders& headers);
    void setTrackingId(const std::string& trackingId);
    void setTimeout(int timeoutSecs);
    void setInFileSizeLarge(size_t file_size);
    void setCommonOptions();

    void finishHandle(CURLcode code);

    void addResponseHeader(const std::string& key, const std::string& val);

    void setRequestDataJsonString(const std::string& jsonString);
    void setRequestDataBuffer(ucf::service::network::http::ByteBufferPtr buffer, ucf::service::network::http::UploadProgressFunction progressFunc);
    void setRequestDataFile(const std::string& filePath, ucf::service::network::http::UploadProgressFunction progressFunc);

    void headersCompleted();
    void appendResponseBody(char *data, size_t size);
    size_t readRequestBody(char *data, size_t size);
    int seekRequestBody(curl_off_t offset, int origin);
public:
    CURL* getHandle() const;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}