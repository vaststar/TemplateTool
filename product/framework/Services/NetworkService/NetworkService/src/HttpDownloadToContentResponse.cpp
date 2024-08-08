#include <ucf/Services/NetworkService/Model/HttpDownloadToContentResponse.h>

namespace ucf::service::network::http{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class HttpDownloadToContentResponse::DataPrivate
{
public:
    DataPrivate();
    void setHttpResponseCode(int statusCode){ mResponseCode = statusCode;}
    int getHttpResponseCode() const{ return mResponseCode;}

    void setResponseHeaders(const NetworkHttpHeaders& headers){ mResponseHeaders = headers;}
    NetworkHttpHeaders getResponseHeaders() const{ return mResponseHeaders;}

    void setErrorData(const ResponseErrorStruct& errorData){ mErrorData = errorData;}
    std::optional<ResponseErrorStruct> getErrorData() const{ return mErrorData;}

    void appendResponseBody(const ByteBuffer& buffer){ mResponseBody.insert(mResponseBody.end(), buffer.begin(), buffer.end());}
    void setResponseBody(const ByteBuffer& buffer){ mResponseBody = buffer;}
    const ByteBuffer& getResponseBody() const{ return mResponseBody;}

    size_t getTotalSize(){ return mTotalSize;}
    void setTotalSize(size_t size){ mTotalSize = size;}
private:
    int mResponseCode;
    NetworkHttpHeaders mResponseHeaders;
    std::optional<ResponseErrorStruct> mErrorData;
    ByteBuffer mResponseBody;
    size_t mTotalSize;
};

HttpDownloadToContentResponse::DataPrivate::DataPrivate()
    : mResponseCode(0)
    , mTotalSize(0)
{

}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start HttpDownloadToContentResponse Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
HttpDownloadToContentResponse::HttpDownloadToContentResponse()
    : mDataPrivate(std::make_unique<HttpDownloadToContentResponse::DataPrivate>())
{

}

HttpDownloadToContentResponse::~HttpDownloadToContentResponse()
{

}


void HttpDownloadToContentResponse::setHttpResponseCode(int statusCode)
{
    mDataPrivate->setHttpResponseCode(statusCode);
}

int HttpDownloadToContentResponse::getHttpResponseCode() const
{
    return mDataPrivate->getHttpResponseCode();
}

void HttpDownloadToContentResponse::setResponseHeaders(const NetworkHttpHeaders& headers)
{
    mDataPrivate->setResponseHeaders(headers);
    auto item = std::find_if(headers.cbegin(), headers.cend(), [](const auto& headerKeyVal){
        return headerKeyVal.first == "Content-Length";
    });
    if (item != headers.cend())
    {
        mDataPrivate->setTotalSize(static_cast<size_t>(std::stoull(item->second)));
    }
}

NetworkHttpHeaders HttpDownloadToContentResponse::getResponseHeaders() const
{
    return mDataPrivate->getResponseHeaders();
}

void HttpDownloadToContentResponse::setErrorData(const ResponseErrorStruct& errorData)
{
    mDataPrivate->setErrorData(errorData);
}

std::optional<ResponseErrorStruct> HttpDownloadToContentResponse::getErrorData() const
{
    return mDataPrivate->getErrorData();
}

void HttpDownloadToContentResponse::appendResponseBody(const ByteBuffer& buffer)
{
    mDataPrivate->appendResponseBody(buffer);
}

void HttpDownloadToContentResponse::setResponseBody(const ByteBuffer& buffer)
{
    mDataPrivate->setResponseBody(buffer);
}

const ByteBuffer& HttpDownloadToContentResponse::getResponseBody() const
{
    return mDataPrivate->getResponseBody();
}

size_t HttpDownloadToContentResponse::getTotalSize() const
{
    return mDataPrivate->getTotalSize();
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start HttpDownloadToContentResponse Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}