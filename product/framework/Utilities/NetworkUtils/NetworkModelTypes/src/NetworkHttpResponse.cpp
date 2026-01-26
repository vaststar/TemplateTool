#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpResponse.h>

#include <algorithm>
#include <cctype>

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
    const NetworkHttpHeaders& getResponseHeaders() const{ return mResponseHeaders;}

    void setErrorData(const ResponseErrorStruct& errorData){ mErrorData = errorData;}
    std::optional<ResponseErrorStruct> getErrorData() const{ return mErrorData;}

    void appendResponseBody(const ByteBuffer& buffer){ mResponseBody.insert(mResponseBody.end(), buffer.begin(), buffer.end());}
    void setResponseBody(const ByteBuffer& buffer){ mResponseBody = buffer;}
    const ByteBuffer& getResponseBody() const{ return mResponseBody;}

    void clear();
private:
    int mResponseCode;
    NetworkHttpHeaders mResponseHeaders;
    std::optional<ResponseErrorStruct> mErrorData;
    ByteBuffer mResponseBody;
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

const NetworkHttpHeaders& NetworkHttpResponse::getResponseHeaders() const
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

void NetworkHttpResponse::appendResponseBody(const ByteBuffer& buffer)
{
    if (!buffer.empty())
    {
        mDataPrivate->appendResponseBody(buffer);
    }
}

void NetworkHttpResponse::setResponseBody(const ByteBuffer& buffer)
{
    mDataPrivate->setResponseBody(buffer);
}

const ByteBuffer& NetworkHttpResponse::getResponseBody() const
{
    return mDataPrivate->getResponseBody();
}

void NetworkHttpResponse::clear()
{
    mDataPrivate->clear();
}

std::optional<std::string> NetworkHttpResponse::getHeaderValue(const std::string& key) const
{
    const auto& headers = getResponseHeaders();
    // HTTP header names are case-insensitive (RFC 7230)
    auto item = std::find_if(headers.cbegin(), headers.cend(), [&key](const auto& headerKeyVal){
        return std::ranges::equal(headerKeyVal.first, key, [](char a, char b){
            return std::tolower(static_cast<unsigned char>(a)) == 
                   std::tolower(static_cast<unsigned char>(b));
        });
    });
    if (item != headers.cend())
    {
        return item->second;
    }
    return {};
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish NetworkHttpResponse Logic/////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}