#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpResponse.h>


#include "NetworkHttpCallbackHandler.h"

namespace ucf::service::network::http{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class NetworkHttpCallbackHandler::DataPrivate{
public:
    DataPrivate();
    void setResponseCallback(const ucf::utilities::network::http::NetworkHttpResponseCallbackFunc& callbackFunc){mResponseCallBack = callbackFunc;}
    ucf::utilities::network::http::NetworkHttpResponseCallbackFunc getResponseCallback() const{return mResponseCallBack;}

    ucf::utilities::network::http::NetworkHttpResponse& getResponse(){return mResponse;}

    void appendBuffer(const ucf::utilities::network::http::ByteBuffer& buffer){mCachedBuffer.insert(mCachedBuffer.end(), buffer.begin(), buffer.end());}
    const ucf::utilities::network::http::ByteBuffer& getCachedBuffer() const{return mCachedBuffer;}
private:
    ucf::utilities::network::http::NetworkHttpResponseCallbackFunc mResponseCallBack;
    ucf::utilities::network::http::NetworkHttpResponse mResponse;
    ucf::utilities::network::http::ByteBuffer mCachedBuffer;
};

NetworkHttpCallbackHandler::DataPrivate::DataPrivate()
{

}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start NetworkHttpCallbackHandler Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
NetworkHttpCallbackHandler::NetworkHttpCallbackHandler()
    : mDataPrivate(std::make_unique<NetworkHttpCallbackHandler::DataPrivate>())
{

}

NetworkHttpCallbackHandler::~NetworkHttpCallbackHandler()
{

}

void NetworkHttpCallbackHandler::setResponseCallback(const ucf::utilities::network::http::NetworkHttpResponseCallbackFunc& callbackFunc)
{
    mDataPrivate->setResponseCallback(callbackFunc);
}

void NetworkHttpCallbackHandler::setResponseHeader(int statusCode, const ucf::utilities::network::http::NetworkHttpHeaders& headers, std::optional<ucf::utilities::network::http::ResponseErrorStruct> errorData)
{
    mDataPrivate->getResponse().setHttpResponseCode(statusCode);
    mDataPrivate->getResponse().setResponseHeaders(headers);
    if (errorData)
    {
        mDataPrivate->getResponse().setErrorData(*errorData);
    }
}

void NetworkHttpCallbackHandler::appendResponseBody(const ucf::utilities::network::http::ByteBuffer& buffer, bool isFinished)
{
    mDataPrivate->appendBuffer(buffer);
    if (isFinished)
    {
        std::string bodyString = std::string(mDataPrivate->getCachedBuffer().begin(), mDataPrivate->getCachedBuffer().end());
        mDataPrivate->getResponse().setResponseBody(std::move(bodyString));
    }
}

void NetworkHttpCallbackHandler::completeResponse(const ucf::utilities::network::http::HttpResponseMetrics& metrics)
{
    if (mDataPrivate->getResponseCallback())
    {
        mDataPrivate->getResponseCallback()(mDataPrivate->getResponse());
    }
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish NetworkHttpCallbackHandler Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}