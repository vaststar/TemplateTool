#include <string>
#include <curl/curl.h>
#include <ucf/NetworkService/NetworkModelTypes/Http/NetworkHttpTypes.h>
#include <ucf/NetworkService/NetworkModelTypes/Http/NetworkHttpResponse.h>


#include "LibCurlEasyHandle.h"
#include "LibCurlClientLogger.h"


namespace ucf::network::libcurl{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class LibCurlEasyHandle::DataPrivate
{
public:
    DataPrivate();
    ~DataPrivate();

    CURL* getHandle();

    ucf::network::http::HttpResponseMetrics getResponseMetrics() const;

    void setCallbacks(const ucf::network::http::HttpHeaderCallback& headerCallback, const ucf::network::http::HttpBodyCallback& bodyCallback, const ucf::network::http::HttpCompletionCallback& completionCallback)
    {
        mHeaderCallback = headerCallback;
        mBodyCallback = bodyCallback;
        mCompletionCallback = completionCallback;
    }
    ucf::network::http::HttpHeaderCallback getHeaderCallback() {return mHeaderCallback;}
    ucf::network::http::HttpBodyCallback getBodyCallback() {return mBodyCallback;}
    ucf::network::http::HttpCompletionCallback getCompletionCallback() {return mCompletionCallback;}

    void setTrackingId(const std::string& trackingId){mTrackingId = trackingId;}
    std::string getTrackingId() const{return mTrackingId;}
private:
    CURL* mHandle;
    ucf::network::http::HttpHeaderCallback mHeaderCallback;
    ucf::network::http::HttpBodyCallback mBodyCallback;
    ucf::network::http::HttpCompletionCallback mCompletionCallback;
    std::string mTrackingId;
};

LibCurlEasyHandle::DataPrivate::DataPrivate()
    : mHandle(curl_easy_init())
{

}

LibCurlEasyHandle::DataPrivate::~DataPrivate()
{
    if (mHandle)
    {
        curl_easy_cleanup(mHandle);
    }
} 

CURL* LibCurlEasyHandle::DataPrivate::getHandle()
{
    return mHandle;
}

ucf::network::http::HttpResponseMetrics LibCurlEasyHandle::DataPrivate::getResponseMetrics() const
{
    curl_off_t nameLookup{0};
    curl_easy_getinfo(mHandle,CURLINFO_NAMELOOKUP_TIME_T, &nameLookup);
    curl_off_t connectTime{0};
    curl_easy_getinfo(mHandle,CURLINFO_CONNECT_TIME_T, &connectTime);
    curl_off_t preTransferTime{0};
    curl_easy_getinfo(mHandle,CURLINFO_PRETRANSFER_TIME_T, &preTransferTime);
    curl_off_t startTransferTime{0};
    curl_easy_getinfo(mHandle,CURLINFO_STARTTRANSFER_TIME_T, &startTransferTime);
    curl_off_t transferTime{0};
    curl_easy_getinfo(mHandle,CURLINFO_TOTAL_TIME_T, &transferTime);
    curl_off_t responseLength{0};
    curl_easy_getinfo(mHandle,CURLINFO_SIZE_DOWNLOAD_T, &responseLength);
    long httpVersion{0};
    curl_easy_getinfo(mHandle,CURLINFO_HTTP_VERSION, &httpVersion);
    
    std::string versionString;
    switch(httpVersion)
    {
        case CURL_HTTP_VERSION_1_0:
            versionString = "HTTP 1.0";
            break;
        case CURL_HTTP_VERSION_1_1:
            versionString = "HTTP 1.1";
            break;
        case CURL_HTTP_VERSION_2_0:
            versionString = "HTTP 2.0";
            break;
        case CURL_HTTP_VERSION_3:
            versionString = "HTTP 3.0";
            break;
        default:
            versionString = "HTTP Unknown";
    }
    
    return { 0, static_cast<int>(nameLookup) / 1000, static_cast<int>(connectTime - nameLookup) / 1000, static_cast<int>(startTransferTime - preTransferTime) / 1000, static_cast<int>(transferTime - startTransferTime) / 1000,  static_cast<size_t>(responseLength), versionString };
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic/////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start LibCurlEasyHandle Logic////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
LibCurlEasyHandle::LibCurlEasyHandle()
    : mDataPrivate(std::make_unique<DataPrivate>())
{

}

LibCurlEasyHandle::~LibCurlEasyHandle()
{
}

CURL* LibCurlEasyHandle::getHandle() const
{
    return mDataPrivate->getHandle();
}

void LibCurlEasyHandle::setCallbacks(const ucf::network::http::HttpHeaderCallback& headerCallback, const ucf::network::http::HttpBodyCallback& bodyCallback, const ucf::network::http::HttpCompletionCallback& completionCallback)
{
    mDataPrivate->setCallbacks(headerCallback, bodyCallback, completionCallback);
}

void LibCurlEasyHandle::setTrackingId(const std::string& trackingId)
{
    mDataPrivate->setTrackingId(trackingId);
}

void LibCurlEasyHandle::finishHandle(CURLcode code)
{
    LIBCURL_LOG_DEBUG("task started to finish trackingId=" << mDataPrivate->getTrackingId());
    if (code == CURLE_OK)
    {
        if (mDataPrivate->getBodyCallback() != nullptr)
        {
            ucf::network::http::ByteBuffer emptyBuf;
            mDataPrivate->getBodyCallback()(emptyBuf, true);
        }

        if (mDataPrivate->getCompletionCallback() != nullptr)
        {
            mDataPrivate->getCompletionCallback()(mDataPrivate->getResponseMetrics());
        }
    }
    else
    {
        LIBCURL_LOG_DEBUG("CURL Error (" << static_cast<int>(code) << "): " << curl_easy_strerror(code) << ", trackingId=" << mDataPrivate->getTrackingId());
        ucf::network::http::NetworkHttpResponse response;

        if (mDataPrivate->getHeaderCallback() != nullptr)
        {
            mDataPrivate->getHeaderCallback()(response);
        }

        if (mDataPrivate->getCompletionCallback() != nullptr)
        {
            mDataPrivate->getCompletionCallback()(mDataPrivate->getResponseMetrics());
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish LibCurlEasyHandle Logic////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}