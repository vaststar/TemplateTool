#include <format>
#include <ucf/Services/NetworkService/NetworkModelTypes/Http/NetworkHttpRequest.h>
#include <ucf/Utilities/UUIDUtils/UUIDUtils.h>

namespace ucf::network::http{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class NetworkHttpRequest::DataPrivate{
public:
    DataPrivate(const HTTPMethod& method, const std::string& uri, const std::map<std::string, std::string>& headers, const std::string& payload);
    std::string getRequestId() const;
    HTTPMethod getMethod() const;
    std::string getUri() const;
    std::string getPayload() const;
    std::map<std::string, std::string> getHeaders() const;
    
    void setTrackingId(const std::string& trackingId);
    std::string getTrackingId() const;
private:
    const std::string mRequestId;
    HTTPMethod mMethod;
    std::string mUri;
    std::map<std::string, std::string> mHeaders;
    std::string mPayload;
    std::string mTrackingId;
};

NetworkHttpRequest::DataPrivate::DataPrivate(const HTTPMethod& method, const std::string& uri, const std::map<std::string, std::string>& headers, const std::string& payload)
    : mRequestId("RequestID_"+ucf::utilities::UUIDUtils::generateUUID())
    , mTrackingId("TrackindID_" + ucf::utilities::UUIDUtils::generateUUID())
    , mMethod(method)
    , mUri(uri)
    , mHeaders(headers)
    , mPayload(payload)
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

std::string NetworkHttpRequest::DataPrivate::getPayload() const
{
    return mPayload;
}

std::map<std::string, std::string> NetworkHttpRequest::DataPrivate::getHeaders() const
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

NetworkHttpRequest::NetworkHttpRequest(const HTTPMethod& method, const std::string& uri, const std::map<std::string, std::string>& headers, const std::string& payload)
    :mDataPrivate(std::make_unique<DataPrivate>(method, uri, headers, payload))
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

std::map<std::string, std::string> NetworkHttpRequest::getRequestHeaders() const
{
    return mDataPrivate->getHeaders();
}

std::string NetworkHttpRequest::getRequestPayload() const
{
    return mDataPrivate->getPayload();
}

std::string NetworkHttpRequest::getRequestId() const
{
    return mDataPrivate->getRequestId();
}

void NetworkHttpRequest::setTrackingId(const std::string& trackingId)
{
    mDataPrivate->setTrackingId(trackingId);
}

std::string NetworkHttpRequest::getTrackingId() const
{
    return mDataPrivate->getTrackingId();
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
        default:
            return "UNKNOWN";
        }};
    return std::format("Http Request, Method:{}, URI:{}, RequestId:{}, TrackingId:{}", getMethodString(), getRequestUri(), getRequestId(), getTrackingId());
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish NetworkHttpRequest Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}