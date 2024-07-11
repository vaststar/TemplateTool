#pragma once

#include <memory>
#include <map>
#include <string>

#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/NetworkModelTypesExport.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpTypes.h>

namespace ucf::utilities::network::http{
class NETWORKTYPE_EXPORT NetworkHttpRequest final
{
public:
    NetworkHttpRequest(const HTTPMethod& method, const std::string& uri, const NetworkHttpHeaders& headers, int timeoutSecs);
    NetworkHttpRequest(const HTTPMethod& method, const std::string& uri, const NetworkHttpHeaders& headers, int timeoutSecs, const std::string& payload);
    NetworkHttpRequest(const HTTPMethod& method, const std::string& uri, const NetworkHttpHeaders& headers, int timeoutSecs, ByteBufferPtr inMemoryBuffer, UploadProgressFunction progressFunc);
    NetworkHttpRequest(const HTTPMethod& method, const std::string& uri, const NetworkHttpHeaders& headers, int timeoutSecs, const std::string& filePath, UploadProgressFunction progressFunc);
    ~NetworkHttpRequest();
    NetworkHttpRequest(const NetworkHttpRequest&) = delete;
    NetworkHttpRequest(NetworkHttpRequest&&) = delete;
    NetworkHttpRequest& operator=(const NetworkHttpRequest&) = delete;
    NetworkHttpRequest& operator=(NetworkHttpRequest&&) = delete;

    std::string getRequestId() const;
    HTTPMethod getRequestMethod() const;
    std::string getRequestUri() const;
    NetworkHttpHeaders getRequestHeaders() const;
    int getTimeout() const;

    NetworkHttpPayloadType getPayloadType() const;
    size_t getPayloadSize() const;
    std::string getPayloadJsonString() const;
    std::string getPayloadFilePath() const;
    ByteBufferPtr getPayloadMemoryBuffer() const;

    UploadProgressFunction getProgressFunction() const;
 
    void setTrackingId(const std::string& trackingId);
    std::string getTrackingId() const;

    std::string toString() const;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}