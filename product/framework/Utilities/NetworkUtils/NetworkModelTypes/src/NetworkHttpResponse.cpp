#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpResponse.h>

namespace ucf::utilities::network::http{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class NetworkHttpResponse::DataPrivate
{
public:
    DataPrivate();
    void setHttpResponseCode(int statusCode){ mResponseCode = statusCode;}
    int getHttpResponseCode() const{ return mResponseCode;}

    void setResponseHeaders(const NetworkHttpHeaders& headers){ mResponseHeaders = headers;}
    NetworkHttpHeaders getResponseHeaders() const{ return mResponseHeaders;}

    void setErrorData(const ResponseErrorStruct& errorData){ mErrorData = errorData;}
    std::optional<ResponseErrorStruct> getErrorData() const{ return mErrorData;}

    void setResponseBody(const std::string& buffer){ mResponseBody = buffer;}
    std::string getResponseBody() const{ return mResponseBody;}

    void clear();
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

void NetworkHttpResponse::DataPrivate::clear()
{
    mResponseCode = 0;
    mResponseHeaders.clear();
    mErrorData = std::nullopt;
    mResponseBody.clear();
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

void NetworkHttpResponse::clear()
{
    mDataPrivate->clear();
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish NetworkHttpResponse Logic/////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}