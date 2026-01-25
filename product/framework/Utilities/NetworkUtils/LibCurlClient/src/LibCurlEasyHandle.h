#pragma once

#include <map>
#include <memory>
#include <curl/curl.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpTypes.h>
namespace ucf::utilities::network::libcurl{
class LibCurlEasyHandle final
{
public:
    LibCurlEasyHandle(ucf::utilities::network::http::HttpHeaderCallback headerCallback, ucf::utilities::network::http::HttpBodyCallback bodyCallback, ucf::utilities::network::http::HttpCompletionCallback completionCallback);
    ~LibCurlEasyHandle();
    LibCurlEasyHandle(const LibCurlEasyHandle&) = delete;
    LibCurlEasyHandle(LibCurlEasyHandle&&) = delete;
    LibCurlEasyHandle& operator=(const LibCurlEasyHandle&) = delete;
    LibCurlEasyHandle& operator=(LibCurlEasyHandle&&) = delete;
public:
    void setHttpMethod(ucf::utilities::network::http::HTTPMethod method);
    void setURI(const std::string& uri);
    void setHeaders(const ucf::utilities::network::http::NetworkHttpHeaders& headers);
    void setTrackingId(const std::string& trackingId);
    void setRequestId(const std::string& requestId);
    std::string getRequestId() const;
    void setTimeout(int timeoutSecs);
    void setInFileSizeLarge(size_t file_size);
    void setCommonOptions();
    void enableCURLDebugPrint();
    void setRequestDataString(const std::string& jsonString);
    void setRequestDataBuffer(ucf::utilities::network::http::ByteBufferPtr buffer, ucf::utilities::network::http::UploadProgressFunction progressFunc);
    void setRequestDataFile(const std::string& filePath, ucf::utilities::network::http::UploadProgressFunction progressFunc);

public:
    size_t readRequestBody(char *data, size_t size);
    int seekRequestBody(curl_off_t offset, int origin);

    void addResponseHeader(const std::string& key, const std::string& val);
    void headersCompleted();
    void appendResponseBody(char *data, size_t size);
    void finishHandle(CURLcode code);
public:
    CURL* getHandle() const;
private:
    ucf::utilities::network::http::ResponseErrorStruct makeErrorData(CURLcode code);
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}