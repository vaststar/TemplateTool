#include <string>
#include <curl/curl.h>
#include <ucf/Utilities/StringUtils/StringUtils.h>
#include <ucf/Services/NetworkService/NetworkModelTypes/Http/NetworkHttpTypes.h>
#include <ucf/Services/NetworkService/NetworkModelTypes/Http/NetworkHttpResponse.h>


#include "LibCurlEasyHandle.h"
#include "LibCurlClientLogger.h"

namespace ucf::network::libcurl{
inline constexpr auto MAX_REQUEST_REDIRECTS{ 5 };

static size_t header_callback(char *data, size_t size, size_t nmemb, void *userp)
{
    if(data == nullptr || userp == nullptr)
    {
        return size * nmemb;
    }
    const auto easyHandle = reinterpret_cast<LibCurlEasyHandle*>(userp);
    const std::string_view headerString(data, size * nmemb);
    const auto seperator = headerString.find(": ");
    if (seperator != std::string::npos)
    {
        const auto key = std::string(headerString.substr(0, seperator));
        const auto value = ucf::utilities::StringUtils::trim(std::string(headerString.substr(seperator + 2)));
        // easyHandle->addResponseHeader(key, value);
    }
    if (headerString == "\r\n") {
        // easyHandle->headersCompleted();
    }
    return size * nmemb;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class LibCurlEasyHandle::DataPrivate
{
public:
    DataPrivate(const ucf::network::http::HttpHeaderCallback& headerCallback, const ucf::network::http::HttpBodyCallback& bodyCallback, const ucf::network::http::HttpCompletionCallback& completionCallback);
    ~DataPrivate();

    CURL* getHandle();

    ucf::network::http::HttpResponseMetrics getResponseMetrics() const;

    ucf::network::http::HttpHeaderCallback getHeaderCallback() {return mHeaderCallback;}
    ucf::network::http::HttpBodyCallback getBodyCallback() {return mBodyCallback;}
    ucf::network::http::HttpCompletionCallback getCompletionCallback() {return mCompletionCallback;}

    void setTrackingId(const std::string& trackingId){mTrackingId = trackingId;}
    std::string getTrackingId() const{return mTrackingId;}

    int setHttpMethod(ucf::network::http::HTTPMethod method);
    int setURI(const std::string& uri);
    int setHeaders(const std::map<std::string, std::string>& headers);
    void setCommonOptions();

    template<typename ...Args>
    int setOption(CURLoption option, const Args& ...args)
    {
        auto const code = curl_easy_setopt(mHandle, option, args...);
        if (code != CURLE_OK)
        {
            LIBCURL_LOG_DEBUG("CURL Error (" << static_cast<int>(code) << "): " << curl_easy_strerror(code));
        }
        return code;
    }
private:
    CURL* mHandle;
    CURLU* mUrl;
    curl_slist* mHeaders;
    ucf::network::http::HttpHeaderCallback mHeaderCallback;
    ucf::network::http::HttpBodyCallback mBodyCallback;
    ucf::network::http::HttpCompletionCallback mCompletionCallback;
    std::string mTrackingId;
};

LibCurlEasyHandle::DataPrivate::DataPrivate(const ucf::network::http::HttpHeaderCallback& headerCallback, const ucf::network::http::HttpBodyCallback& bodyCallback, const ucf::network::http::HttpCompletionCallback& completionCallback)
    : mHandle(curl_easy_init())
    , mUrl(curl_url())
    , mHeaders(nullptr)
    , mHeaderCallback(headerCallback)
    , mBodyCallback(bodyCallback)
    , mCompletionCallback(completionCallback)
{

}

LibCurlEasyHandle::DataPrivate::~DataPrivate()
{
    if (mHandle)
    {
        curl_easy_cleanup(mHandle);
    }
    if (mUrl)
    {
        curl_url_cleanup(mUrl);
    }
    if (mHeaders)
    {
        curl_slist_free_all(mHeaders);
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

int LibCurlEasyHandle::DataPrivate::setHttpMethod(ucf::network::http::HTTPMethod method)
{
    int result = { 0 };
    switch (method)
    {
        case ucf::network::http::HTTPMethod::GET:
            result = setOption(CURLOPT_HTTPGET, 1L);
            break;
            break;
        case ucf::network::http::HTTPMethod::POST:
            setOption(CURLOPT_POST, 1L);
            result = setOption(CURLOPT_POSTFIELDS, nullptr);
            break;
        case ucf::network::http::HTTPMethod::HEAD:
            setOption(CURLOPT_NOBODY, 1L);
            result = setOption(CURLOPT_CUSTOMREQUEST, "HEAD");
            break;
        case ucf::network::http::HTTPMethod::PUT:
            setOption(CURLOPT_CUSTOMREQUEST, "PUT");
            result = setOption(CURLOPT_POSTFIELDS, nullptr);
            break;
        case ucf::network::http::HTTPMethod::DEL:
            result = setOption(CURLOPT_CUSTOMREQUEST, "DELETE");
            break;
        case ucf::network::http::HTTPMethod::PATCH:
            result = setOption(CURLOPT_CUSTOMREQUEST, "PATCH");
            break;
        case ucf::network::http::HTTPMethod::OPTIONS:
            result = setOption(CURLOPT_CUSTOMREQUEST, "OPTIONS");
            break;
        default:
            LIBCURL_LOG_DEBUG("Unknown HTTP Method");
    }
    return result;
}

int LibCurlEasyHandle::DataPrivate::setURI(const std::string& uri)
{
    auto const code = curl_url_set(mUrl, CURLUPART_URL, uri.c_str(), 0);

    if (code != CURLUE_OK)
    {
        LIBCURL_LOG_WARN("Error parsing url: " << static_cast<int>(code));
    }

    return code;
}

int LibCurlEasyHandle::DataPrivate::setHeaders(const std::map<std::string, std::string>& headers)
{
    for (auto[key, val] : headers)
    {
        auto header = key + ": " + val;
        mHeaders = curl_slist_append(mHeaders, header.c_str());
    }
    if (!mHeaders)
    {
        LIBCURL_LOG_WARN("Error setting headers");
    }
    return setOption(CURLOPT_HTTPHEADER, mHeaders);
}

void LibCurlEasyHandle::DataPrivate::setCommonOptions()
{
    setOption(CURLOPT_HEADERFUNCTION, header_callback);
    //setOption(CURLOPT_WRITEFUNCTION, response_body_callback);
    setOption(CURLOPT_MAXREDIRS, MAX_REQUEST_REDIRECTS);
    setOption(CURLOPT_TRANSFER_ENCODING, 1L);
    setOption(CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
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
LibCurlEasyHandle::LibCurlEasyHandle(const ucf::network::http::HttpHeaderCallback& headerCallback, const ucf::network::http::HttpBodyCallback& bodyCallback, const ucf::network::http::HttpCompletionCallback& completionCallback)
    : mDataPrivate(std::make_unique<DataPrivate>(headerCallback, bodyCallback, completionCallback))
{

}

LibCurlEasyHandle::~LibCurlEasyHandle()
{
}

CURL* LibCurlEasyHandle::getHandle() const
{
    return mDataPrivate->getHandle();
}

void LibCurlEasyHandle::setHttpMethod(ucf::network::http::HTTPMethod method)
{
    mDataPrivate->setHttpMethod(method);
}

void LibCurlEasyHandle::setURI(const std::string& uri)
{
    mDataPrivate->setURI(uri);
}

void LibCurlEasyHandle::setHeaders(const std::map<std::string, std::string>& headers)
{
    mDataPrivate->setHeaders(headers);
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