#include <format>

#include <ucf/Services/NetworkService/NetworkModelTypes/Http/NetworkHttpRequest.h>
#include <ucf/Utilities/UUIDUtils/UUIDUtils.h>

namespace ucf::service::network::http{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class NetworkHttpRequest::DataPrivate{
public:
    DataPrivate(const HTTPMethod& method, const std::string& uri, const NetworkHttpHeaders& headers, int timeoutSecs);
    DataPrivate(const HTTPMethod& method, const std::string& uri, const NetworkHttpHeaders& headers, const std::string& payload, int timeoutSecs);
    std::string getRequestId() const;
    HTTPMethod getMethod() const;
    std::string getUri() const;
    NetworkHttpPayloadType getPayloadType() const;
    std::string getJsonPayload() const;
    NetworkHttpHeaders getHeaders() const;
    int getTimeoutSecs() const;
    
    void setTrackingId(const std::string& trackingId);
    std::string getTrackingId() const;
private:
    const std::string mRequestId;
    HTTPMethod mMethod;
    std::string mUri;
    NetworkHttpHeaders mHeaders;
    NetworkHttpPayloadType mPayloadType;
    std::string mPayload;
    int mTimeoutSecs;
    std::string mTrackingId;
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

NetworkHttpRequest::DataPrivate::DataPrivate(const HTTPMethod& method, const std::string& uri, const NetworkHttpHeaders& headers, const std::string& payload, int timeoutSecs)
    : mRequestId("RequestID_"+ucf::utilities::UUIDUtils::generateUUID())
    , mTrackingId("TrackindID_" + ucf::utilities::UUIDUtils::generateUUID())
    , mMethod(method)
    , mUri(uri)
    , mHeaders(headers)
    , mPayloadType(NetworkHttpPayloadType::Json)
    , mPayload(payload)
    , mTimeoutSecs(timeoutSecs)
{

}

NetworkHttpRequest::~NetworkHttpRequest()
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

std::string NetworkHttpRequest::DataPrivate::getJsonPayload() const
{
    return mPayload;
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

NetworkHttpRequest::NetworkHttpRequest(const HTTPMethod& method, const std::string& uri, const NetworkHttpHeaders& headers, const std::string& payload, int timeoutSecs)
    :mDataPrivate(std::make_unique<DataPrivate>(method, uri, headers, payload, timeoutSecs))
{

}

NetworkHttpRequest::NetworkHttpRequest(const HTTPMethod& method, const std::string& uri, const NetworkHttpHeaders& headers, int timeoutSecs)
    :mDataPrivate(std::make_unique<DataPrivate>(method, uri, headers, timeoutSecs))
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

std::string NetworkHttpRequest::getJsonPayload() const
{
    return mDataPrivate->getJsonPayload();
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