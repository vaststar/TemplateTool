#include <ucf/Services/NetworkService/NetworkModelTypes/Http/NetworkHttpResponse.h>

namespace ucf::service::network::http{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class NetworkHttpResponse::DataPrivate
{
public:
    DataPrivate();
    void setHttpResponseCode(int statusCode);
    int getHttpResponseCode() const;

    void setResponseHeaders(const NetworkHttpHeaders& headers);
    NetworkHttpHeaders getResponseHeaders() const;

    void setErrorData(const ResponseErrorStruct& errorData);
    std::optional<ResponseErrorStruct> getErrorData() const;

    void setResponseBody(const std::string& buffer);
    std::string getResponseBody() const;
private:
    int mResponseCode;
    NetworkHttpHeaders mResponseHeaders;
    std::optional<ResponseErrorStruct> mErrorData;
    std::string mResponseBody;
};

NetworkHttpResponse::DataPrivate::DataPrivate()
    : mResponseCode(0)
    , mErrorData(std::nullopt)
{

}

void NetworkHttpResponse::DataPrivate::setHttpResponseCode(int statusCode)
{
    mResponseCode = statusCode;
}

int NetworkHttpResponse::DataPrivate::getHttpResponseCode() const
{
    return mResponseCode;
}

void NetworkHttpResponse::DataPrivate::setResponseHeaders(const NetworkHttpHeaders& headers)
{
    mResponseHeaders = headers;
}

NetworkHttpHeaders NetworkHttpResponse::DataPrivate::getResponseHeaders() const
{
    return mResponseHeaders;
}

void NetworkHttpResponse::DataPrivate::setErrorData(const ResponseErrorStruct& errorData)
{
    mErrorData = errorData;
}

std::optional<ResponseErrorStruct> NetworkHttpResponse::DataPrivate::getErrorData() const
{
    return mErrorData;
}


void NetworkHttpResponse::DataPrivate::setResponseBody(const std::string& buffer)
{
    mResponseBody = buffer;
}

std::string NetworkHttpResponse::DataPrivate::getResponseBody() const
{
    return mResponseBody;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic/////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start NetworkHttpResponse Logic//////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
NetworkHttpResponse::NetworkHttpResponse()
    : mDataPrivate(std::make_unique<DataPrivate>())
{

}

NetworkHttpResponse::~NetworkHttpResponse()
{

}

void NetworkHttpResponse::setHttpResponseCode(int statusCode)
{
    mDataPrivate->setHttpResponseCode(statusCode);
}

int NetworkHttpResponse::getHttpResponseCode() const
{
    return mDataPrivate->getHttpResponseCode();
}

void NetworkHttpResponse::setResponseHeaders(const NetworkHttpHeaders& headers)
{
    mDataPrivate->setResponseHeaders(headers);
}

NetworkHttpHeaders NetworkHttpResponse::getResponseHeaders() const
{
    return mDataPrivate->getResponseHeaders();
}

void NetworkHttpResponse::setErrorData(const ResponseErrorStruct& errorData)
{
    mDataPrivate->setErrorData(errorData);
}

std::optional<ResponseErrorStruct> NetworkHttpResponse::getErrorData() const
{
    return mDataPrivate->getErrorData();
}

void NetworkHttpResponse::setResponseBody(const std::string& buffer)
{
    mDataPrivate->setResponseBody(buffer);
}

std::string NetworkHttpResponse::getResponseBody() const
{
    return mDataPrivate->getResponseBody();
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish NetworkHttpResponse Logic/////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}