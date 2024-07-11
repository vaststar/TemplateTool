#include <format>
#include <variant>
#include <filesystem>

#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpRequest.h>
#include <ucf/Utilities/UUIDUtils/UUIDUtils.h>

namespace ucf::utilities::network::http{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class NetworkHttpRequest::DataPrivate{
public:
    DataPrivate(const HTTPMethod& method, const std::string& uri, const NetworkHttpHeaders& headers, int timeoutSecs);
    DataPrivate(const HTTPMethod& method, const std::string& uri, const NetworkHttpHeaders& headers, int timeoutSecs, const std::string& payload);
    DataPrivate(const HTTPMethod& method, const std::string& uri, const NetworkHttpHeaders& headers, int timeoutSecs, ByteBufferPtr inMemoryBuffer, UploadProgressFunction progressFunc);
    DataPrivate(const HTTPMethod& method, const std::string& uri, const NetworkHttpHeaders& headers, int timeoutSecs, const std::string& filePath, UploadProgressFunction progressFunc);
    std::string getRequestId() const;
    HTTPMethod getMethod() const;
    std::string getUri() const;
    NetworkHttpHeaders getHeaders() const;
    int getTimeoutSecs() const;
    
    void setTrackingId(const std::string& trackingId);
    std::string getTrackingId() const;

    NetworkHttpPayloadType getPayloadType() const;
    std::string getPayloadJsonString() const;
    std::string getPayloadFilePath() const;
    ByteBufferPtr getPayloadMemoryBuffer() const;
    UploadProgressFunction getProgressFunction() const;
private:
    const std::string mRequestId;
    HTTPMethod mMethod;
    std::string mUri;
    NetworkHttpHeaders mHeaders;
    int mTimeoutSecs;
    std::string mTrackingId;

    NetworkHttpPayloadType mPayloadType;
    std::string mPayloadJson;
    std::string mPayloadFilePath;
    ByteBufferPtr mPayloadBuffer;
    UploadProgressFunction mUploadProgressFunction;
};

NetworkHttpRequest::DataPrivate::DataPrivate(const HTTPMethod& method, const std::string& uri, const NetworkHttpHeaders& headers, int timeoutSecs)
    : mRequestId("RequestID_"+ucf::utilities::UUIDUtils::generateUUID())
    , mTrackingId("TrackindID_" + ucf::utilities::UUIDUtils::generateUUID())
    , mMethod(method)
    , mUri(uri)
    , mHeaders(headers)
    , mPayloadType(NetworkHttpPayloadType::None)
    , mTimeoutSecs(timeoutSecs)
{

}

NetworkHttpRequest::DataPrivate::DataPrivate(const HTTPMethod& method, const std::string& uri, const NetworkHttpHeaders& headers, int timeoutSecs, const std::string& payload)
    : mRequestId("RequestID_"+ucf::utilities::UUIDUtils::generateUUID())
    , mTrackingId("TrackindID_" + ucf::utilities::UUIDUtils::generateUUID())
    , mMethod(method)
    , mUri(uri)
    , mHeaders(headers)
    , mPayloadType(NetworkHttpPayloadType::Json)
    , mPayloadJson(payload)
    , mTimeoutSecs(timeoutSecs)
{

}

NetworkHttpRequest::DataPrivate::DataPrivate(const HTTPMethod& method, const std::string& uri, const NetworkHttpHeaders& headers, int timeoutSecs, ByteBufferPtr inMemoryBuffer, UploadProgressFunction progressFunc)
    : mRequestId("RequestID_"+ucf::utilities::UUIDUtils::generateUUID())
    , mTrackingId("TrackindID_" + ucf::utilities::UUIDUtils::generateUUID())
    , mMethod(method)
    , mUri(uri)
    , mHeaders(headers)
    , mPayloadType(NetworkHttpPayloadType::Memory)
    , mPayloadBuffer(inMemoryBuffer)
    , mUploadProgressFunction(progressFunc)
    , mTimeoutSecs(timeoutSecs)
{

}

NetworkHttpRequest::DataPrivate::DataPrivate(const HTTPMethod& method, const std::string& uri, const NetworkHttpHeaders& headers, int timeoutSecs, const std::string& filePath, UploadProgressFunction progressFunc)
    : mRequestId("RequestID_"+ucf::utilities::UUIDUtils::generateUUID())
    , mTrackingId("TrackindID_" + ucf::utilities::UUIDUtils::generateUUID())
    , mMethod(method)
    , mUri(uri)
    , mHeaders(headers)
    , mPayloadType(NetworkHttpPayloadType::File)
    , mPayloadFilePath(filePath)
    , mUploadProgressFunction(progressFunc)
    , mTimeoutSecs(timeoutSecs)
{

}

std::string NetworkHttpRequest::DataPrivate::getRequestId() const
{
    return mRequestId;
}

HTTPMethod NetworkHttpRequest::DataPrivate::getMethod() const
{
    return mMethod;
}

std::string NetworkHttpRequest::DataPrivate::getUri() const
{
    return mUri;
}

NetworkHttpPayloadType NetworkHttpRequest::DataPrivate::getPayloadType() const
{
    return mPayloadType;
}

std::string NetworkHttpRequest::DataPrivate::getPayloadJsonString() const
{
    return mPayloadJson;
}

std::string NetworkHttpRequest::DataPrivate::getPayloadFilePath() const
{
    return mPayloadFilePath;
}

ByteBufferPtr NetworkHttpRequest::DataPrivate::getPayloadMemoryBuffer() const
{
    return mPayloadBuffer;
}

UploadProgressFunction NetworkHttpRequest::DataPrivate::getProgressFunction() const
{
    return mUploadProgressFunction;
}

NetworkHttpHeaders NetworkHttpRequest::DataPrivate::getHeaders() const
{
    return mHeaders;
}

void NetworkHttpRequest::DataPrivate::setTrackingId(const std::string& trackingId)
{
    mTrackingId = trackingId;
}

std::string NetworkHttpRequest::DataPrivate::getTrackingId() const
{
    return mTrackingId;
}

int NetworkHttpRequest::DataPrivate::getTimeoutSecs() const
{
    return mTimeoutSecs;
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start NetworkHttpRequest Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

NetworkHttpRequest::NetworkHttpRequest(const HTTPMethod& method, const std::string& uri, const NetworkHttpHeaders& headers, int timeoutSecs, const std::string& payload)
    :mDataPrivate(std::make_unique<DataPrivate>(method, uri, headers, timeoutSecs, payload))
{

}

NetworkHttpRequest::NetworkHttpRequest(const HTTPMethod& method, const std::string& uri, const NetworkHttpHeaders& headers, int timeoutSecs)
    :mDataPrivate(std::make_unique<DataPrivate>(method, uri, headers, timeoutSecs))
{

}


NetworkHttpRequest::NetworkHttpRequest(const HTTPMethod& method, const std::string& uri, const NetworkHttpHeaders& headers, int timeoutSecs, ByteBufferPtr inMemoryBuffer, UploadProgressFunction progressFunc)
    :mDataPrivate(std::make_unique<DataPrivate>(method, uri, headers, timeoutSecs, inMemoryBuffer, progressFunc))
{

}

NetworkHttpRequest::NetworkHttpRequest(const HTTPMethod& method, const std::string& uri, const NetworkHttpHeaders& headers, int timeoutSecs, const std::string& filePath, UploadProgressFunction progressFunc)
    :mDataPrivate(std::make_unique<DataPrivate>(method, uri, headers, timeoutSecs, filePath, progressFunc))
{

}

NetworkHttpRequest::~NetworkHttpRequest()
{

}

HTTPMethod NetworkHttpRequest::getRequestMethod() const
{
    return mDataPrivate->getMethod();
}

std::string NetworkHttpRequest::getRequestUri() const
{
    return mDataPrivate->getUri();
}

NetworkHttpHeaders NetworkHttpRequest::getRequestHeaders() const
{
    return mDataPrivate->getHeaders();
}

std::string NetworkHttpRequest::getRequestId() const
{
    return mDataPrivate->getRequestId();
}

int NetworkHttpRequest::getTimeout() const
{
    return mDataPrivate->getTimeoutSecs();
}

void NetworkHttpRequest::setTrackingId(const std::string& trackingId)
{
    mDataPrivate->setTrackingId(trackingId);
}

std::string NetworkHttpRequest::getTrackingId() const
{
    return mDataPrivate->getTrackingId();
}

NetworkHttpPayloadType NetworkHttpRequest::getPayloadType() const
{
    return mDataPrivate->getPayloadType();
}

size_t NetworkHttpRequest::getPayloadSize() const
{
    switch (getPayloadType())
    {
    case NetworkHttpPayloadType::Json:
        return getPayloadJsonString().size();
    case NetworkHttpPayloadType::Memory:
        return getPayloadMemoryBuffer()->size();
    case NetworkHttpPayloadType::File:
        return std::filesystem::file_size(getPayloadFilePath());
    default:
        return 0;
    }
}

std::string NetworkHttpRequest::getPayloadJsonString() const
{
    return mDataPrivate->getPayloadJsonString();
}

std::string NetworkHttpRequest::getPayloadFilePath() const
{
    return mDataPrivate->getPayloadFilePath();
}

ByteBufferPtr NetworkHttpRequest::getPayloadMemoryBuffer() const
{
    return mDataPrivate->getPayloadMemoryBuffer();
}

UploadProgressFunction NetworkHttpRequest::getProgressFunction() const
{
    return mDataPrivate->getProgressFunction();
}

std::string NetworkHttpRequest::toString() const
{
    auto getMethodString = [this]() {
        switch (getRequestMethod())
        {
        case HTTPMethod::GET:
            return "GET";
        case HTTPMethod::POST:
            return "POST";
        case HTTPMethod::HEAD:
            return "HEAD";
        case HTTPMethod::PUT:
            return "PUT";
        case HTTPMethod::DEL:
            return "DEL";
        case HTTPMethod::PATCH:
            return "PATCH";
        case HTTPMethod::OPTIONS:
            return "OPTIONS";
        default:
            return "UNKNOWN";
        }};
    return "NetworkHttpRequest, {" + std::format("\"Method\": \"{}\", \"URI\": \"{}\", \"RequestId\": \"{}\", \"TrackingId\": \"{}\"", getMethodString(), getRequestUri(), getRequestId(), getTrackingId()) + "}";
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish NetworkHttpRequest Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}