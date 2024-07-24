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
    NetworkHttpRequest();
    ~NetworkHttpRequest();
    NetworkHttpRequest(const NetworkHttpRequest&) = delete;
    NetworkHttpRequest(NetworkHttpRequest&&) = delete;
    NetworkHttpRequest& operator=(const NetworkHttpRequest&) = delete;
    NetworkHttpRequest& operator=(NetworkHttpRequest&&) = delete;

    //requestID could be autofilled
    void setRequestId(const std::string& requestId);
    std::string getRequestId() const;

    //trackingID could be autofilled
    void setTrackingId(const std::string& trackingId);
    std::string getTrackingId() const;

    void setRequestMethod(const ucf::utilities::network::http::HTTPMethod& httpMethod);
    ucf::utilities::network::http::HTTPMethod getRequestMethod() const;

    void setRequestUri(const std::string& requestUri);
    std::string getRequestUri() const;

    void setRequestHeaders(const NetworkHttpHeaders& headers);
    NetworkHttpHeaders getRequestHeaders() const;

    void setTimeout(int timeout);
    int getTimeout() const;

    NetworkHttpPayloadType getPayloadType() const;
    size_t getPayloadSize() const;

    void setPayloadString(const std::string& stringPayload);
    std::string getPayloadString() const;

    void setPayloadFilePath(const std::string& filePath);
    std::string getPayloadFilePath() const;

    void setPayloadMemoryBuffer(ByteBufferPtr byteBuffer);
    ByteBufferPtr getPayloadMemoryBuffer() const;

    void setProgressFunction(UploadProgressFunction progressFunc);
    UploadProgressFunction getProgressFunction() const;

    std::string toString() const;
    void clear();
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}