#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <curl/curl.h>
#include <ucf/Utilities/StringUtils/StringUtils.h>
#include <ucf/Services/NetworkService/NetworkModelTypes/Http/NetworkHttpTypes.h>
#include <ucf/Services/NetworkService/NetworkModelTypes/Http/NetworkHttpResponse.h>


#include "LibCurlEasyHandle.h"
#include "LibCurlClientLogger.h"
#include "LibCurlPayloadData.h"

namespace ucf::service::network::libcurl{
inline constexpr auto MAX_REQUEST_REDIRECTS{ 5 };

static size_t header_callback(char *data, size_t size, size_t nmemb, void *userp)
{
    if(data == nullptr || userp == nullptr)
    {
        return size * nmemb;
    }
    const auto easyHandle = reinterpret_cast<LibCurlEasyHandle*>(userp);
    const std::string_view headerString(data, size * nmemb);
    if (const auto seperator = headerString.find(": "); seperator != std::string::npos)
    {
        const auto key = std::string(headerString.substr(0, seperator));
        const auto value = ucf::utilities::StringUtils::trim(std::string(headerString.substr(seperator + 2)));
        easyHandle->addResponseHeader(key, value);
    }
    if (headerString == "\r\n") {
        easyHandle->headersCompleted();
    }
    return size * nmemb;
}

static size_t response_body_callback(char *data, size_t size, size_t nmemb, void *userp)
{
    if(data == nullptr || userp == nullptr)
    {
        return size * nmemb;
    }
    const auto easyHandle = reinterpret_cast<LibCurlEasyHandle*>(userp);
    easyHandle->appendResponseBody(data, size * nmemb);
    return size * nmemb;
}

static size_t request_body_callback(char *data, size_t size, size_t nmemb, void *userp)
{
    if (userp == nullptr)
    {
        LIBCURL_LOG_ERROR("user data is null");
        return 0;
    }
    const auto easyHandle = reinterpret_cast<LibCurlEasyHandle*>(userp);
    return easyHandle->readRequestBody(data, size * nmemb);
}

int seek_body_callback(void *userp, curl_off_t offset, int origin)
{
    if (userp == nullptr)
    {
        LIBCURL_LOG_ERROR("user data is null");
        return CURL_SEEKFUNC_CANTSEEK;
    }
    const auto easyHandle = reinterpret_cast<LibCurlEasyHandle*>(userp);
    return easyHandle->seekRequestBody(offset, origin);

}

static int debug_callback(CURL *handle, curl_infotype type, char *data, size_t size, void *userp)
{
    if (type == CURLINFO_SSL_DATA_IN || type == CURLINFO_SSL_DATA_OUT)
    {
        return 0;
    }

    std::string typeStr;
    switch (type)
    {
    case CURLINFO_TEXT:
        typeStr = "TEXT";
        break;
    case CURLINFO_HEADER_IN:
        typeStr = "HEADER_IN";
        break;
    case CURLINFO_HEADER_OUT:
        typeStr = "HEADER_OUT";
        break;
    case CURLINFO_DATA_IN:
        typeStr = "DATA_IN";
        break;
    case CURLINFO_DATA_OUT:
        typeStr = "DATA_OUT";
        break;
    default:
        typeStr = "UNHANDLE";
        break;
    }
    std::string dataStr;
    dataStr.append(data, size);
    if (type == CURLINFO_HEADER_OUT)
    {
        std::istringstream f(dataStr);
        std::string line;
        while(std::getline(f, line, '\n'))
        {
            size_t last_pos = line.size()?line.size()-1:0;
            if(line[last_pos] == '\r')
            {
                line[last_pos] = 0;
                LIBCURL_LOG_DEBUG("info:["<<typeStr<<"]"<< line);
            }

        }
    }
    else
    {
        for(auto it = dataStr.rbegin(); it!=dataStr.rend();++it)
        {
            if (*it == '\r'||*it=='\n')
            {
                *it = 0;
            }
        }
        LIBCURL_LOG_DEBUG("info:["<<typeStr<<"]"<<dataStr);
    }
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class LibCurlEasyHandle::DataPrivate
{
public:
    DataPrivate(const ucf::service::network::http::HttpHeaderCallback& headerCallback, const ucf::service::network::http::HttpBodyCallback& bodyCallback, const ucf::service::network::http::HttpCompletionCallback& completionCallback);
    ~DataPrivate();

    CURL* getHandle();

    ucf::service::network::http::HttpResponseMetrics getResponseMetrics() const;

    ucf::service::network::http::HttpHeaderCallback getHeaderCallback() {return mHeaderCallback;}
    ucf::service::network::http::HttpBodyCallback getBodyCallback() {return mBodyCallback;}
    ucf::service::network::http::HttpCompletionCallback getCompletionCallback() {return mCompletionCallback;}

    void setTrackingId(const std::string& trackingId){mTrackingId = trackingId;}
    std::string getTrackingId() const{return mTrackingId;}

    int setHttpMethod(ucf::service::network::http::HTTPMethod method);
    int setURI(const std::string& uri);
    int setHeaders(const ucf::service::network::http::NetworkHttpHeaders& headers);

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

    template<typename ...Args>
    int getInfo(CURLINFO info, Args ...args) const
    {
        auto const code = curl_easy_getinfo(mHandle, info, args...);
        if (code != CURLE_OK)
        {
            LIBCURL_LOG_DEBUG("CURL Error (" << static_cast<int>(code) << "): " << curl_easy_strerror(code));
        }
        return code;
    }
    void addResponseHeader(const std::string& key, const std::string& val);
    ucf::service::network::http::NetworkHttpHeaders getResponseHeader() const;

    int getResponseCode();

    void setPayloadData(std::shared_ptr<PayloadData> payload);
    std::shared_ptr<PayloadData> getPayloadData() const;
private:
    CURL* mHandle;
    CURLU* mUrl;
    curl_slist* mHeaders;
    ucf::service::network::http::HttpHeaderCallback mHeaderCallback;
    ucf::service::network::http::HttpBodyCallback mBodyCallback;
    ucf::service::network::http::HttpCompletionCallback mCompletionCallback;
    std::string mTrackingId;

    ucf::service::network::http::NetworkHttpHeaders mResponseHeader;
    std::shared_ptr<PayloadData> mPayloadData;
};

LibCurlEasyHandle::DataPrivate::DataPrivate(const ucf::service::network::http::HttpHeaderCallback& headerCallback, const ucf::service::network::http::HttpBodyCallback& bodyCallback, const ucf::service::network::http::HttpCompletionCallback& completionCallback)
    : mHandle(curl_easy_init())
    , mUrl(curl_url())
    , mHeaders(nullptr)
    , mHeaderCallback(headerCallback)
    , mBodyCallback(bodyCallback)
    , mCompletionCallback(completionCallback)
    , mPayloadData(nullptr)
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

ucf::service::network::http::HttpResponseMetrics LibCurlEasyHandle::DataPrivate::getResponseMetrics() const
{
    curl_off_t nameLookup{0};
    getInfo(CURLINFO_NAMELOOKUP_TIME_T, &nameLookup);
    curl_off_t connectTime{0};
    getInfo(CURLINFO_CONNECT_TIME_T, &connectTime);
    curl_off_t preTransferTime{0};
    getInfo(CURLINFO_PRETRANSFER_TIME_T, &preTransferTime);
    curl_off_t startTransferTime{0};
    getInfo(CURLINFO_STARTTRANSFER_TIME_T, &startTransferTime);
    curl_off_t transferTime{0};
    getInfo(CURLINFO_TOTAL_TIME_T, &transferTime);
    curl_off_t responseLength{0};
    getInfo(CURLINFO_SIZE_DOWNLOAD_T, &responseLength);
    long httpVersion{0};
    getInfo(CURLINFO_HTTP_VERSION, &httpVersion);
    
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

int LibCurlEasyHandle::DataPrivate::setHttpMethod(ucf::service::network::http::HTTPMethod method)
{
    int result = { 0 };
    switch (method)
    {
        case ucf::service::network::http::HTTPMethod::GET:
            result = setOption(CURLOPT_HTTPGET, 1L);
            break;
        case ucf::service::network::http::HTTPMethod::POST:
            setOption(CURLOPT_POST, 1L);
            result = setOption(CURLOPT_POSTFIELDS, nullptr);
            break;
        case ucf::service::network::http::HTTPMethod::HEAD:
            setOption(CURLOPT_NOBODY, 1L);
            result = setOption(CURLOPT_CUSTOMREQUEST, "HEAD");
            break;
        case ucf::service::network::http::HTTPMethod::PUT:
            setOption(CURLOPT_CUSTOMREQUEST, "PUT");
            result = setOption(CURLOPT_POSTFIELDS, nullptr);
            break;
        case ucf::service::network::http::HTTPMethod::DEL:
            result = setOption(CURLOPT_CUSTOMREQUEST, "DELETE");
            break;
        case ucf::service::network::http::HTTPMethod::PATCH:
            result = setOption(CURLOPT_CUSTOMREQUEST, "PATCH");
            break;
        case ucf::service::network::http::HTTPMethod::OPTIONS:
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
        LIBCURL_LOG_WARN("Error parsing url: " << curl_url_strerror(code));
    }

    setOption(CURLOPT_CURLU, mUrl);

    return code;
}

int LibCurlEasyHandle::DataPrivate::setHeaders(const ucf::service::network::http::NetworkHttpHeaders& headers)
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

void LibCurlEasyHandle::DataPrivate::addResponseHeader(const std::string& key, const std::string& val)
{
    mResponseHeader.emplace_back(key, val);
}

ucf::service::network::http::NetworkHttpHeaders LibCurlEasyHandle::DataPrivate::getResponseHeader() const
{
    return mResponseHeader;
}

int LibCurlEasyHandle::DataPrivate::getResponseCode()
{
    long responseCode = 0;
    getInfo(CURLINFO_RESPONSE_CODE, &responseCode);
    return responseCode;
}

void LibCurlEasyHandle::DataPrivate::setPayloadData(std::shared_ptr<PayloadData> payload)
{
    mPayloadData = payload;
}

std::shared_ptr<PayloadData> LibCurlEasyHandle::DataPrivate::getPayloadData() const
{
    return mPayloadData;
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
LibCurlEasyHandle::LibCurlEasyHandle(const ucf::service::network::http::HttpHeaderCallback& headerCallback, const ucf::service::network::http::HttpBodyCallback& bodyCallback, const ucf::service::network::http::HttpCompletionCallback& completionCallback)
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

void LibCurlEasyHandle::setHttpMethod(ucf::service::network::http::HTTPMethod method)
{
    mDataPrivate->setHttpMethod(method);
}

void LibCurlEasyHandle::setURI(const std::string& uri)
{
    mDataPrivate->setURI(uri);
}

void LibCurlEasyHandle::setHeaders(const ucf::service::network::http::NetworkHttpHeaders& headers)
{
    mDataPrivate->setHeaders(headers);
}

void LibCurlEasyHandle::setTrackingId(const std::string& trackingId)
{
    mDataPrivate->setTrackingId(trackingId);
    // mDataPrivate->setOption(CURLOPT_DEBUGDATA, timeoutSecs);
}

void LibCurlEasyHandle::setTimeout(int timeoutSecs)
{
    mDataPrivate->setOption(CURLOPT_TIMEOUT, timeoutSecs);
}

void LibCurlEasyHandle::setInFileSizeLarge(size_t file_size)
{
    mDataPrivate->setOption(CURLOPT_INFILESIZE_LARGE, file_size);
}

void LibCurlEasyHandle::setCommonOptions()
{    
    mDataPrivate->setOption(CURLOPT_READFUNCTION, request_body_callback);
    mDataPrivate->setOption(CURLOPT_READDATA, this);
    mDataPrivate->setOption(CURLOPT_SEEKFUNCTION, seek_body_callback);
    mDataPrivate->setOption(CURLOPT_SEEKDATA, this);

    mDataPrivate->setOption(CURLOPT_HEADERFUNCTION, header_callback);
    mDataPrivate->setOption(CURLOPT_HEADERDATA, this);
    mDataPrivate->setOption(CURLOPT_WRITEFUNCTION, response_body_callback);
    mDataPrivate->setOption(CURLOPT_WRITEDATA, this);

    mDataPrivate->setOption(CURLOPT_MAXREDIRS, MAX_REQUEST_REDIRECTS);
    mDataPrivate->setOption(CURLOPT_TRANSFER_ENCODING, 1L);
    mDataPrivate->setOption(CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    mDataPrivate->setOption(CURLOPT_SSL_VERIFYPEER, 0L);
    mDataPrivate->setOption(CURLOPT_SSL_VERIFYHOST, 0L);

    mDataPrivate->setOption(CURLOPT_VERBOSE, 1L);
    mDataPrivate->setOption(CURLOPT_DEBUGFUNCTION, debug_callback);
    mDataPrivate->setOption(CURLOPT_NOSIGNAL, 1L);
}

void LibCurlEasyHandle::addResponseHeader(const std::string& key, const std::string& val)
{
    mDataPrivate->addResponseHeader(key, val);
}

void LibCurlEasyHandle::appendResponseBody(char *data, size_t size)
{
    if (mDataPrivate->getBodyCallback() != nullptr)
    {
        mDataPrivate->getBodyCallback()(ucf::service::network::http::ByteBuffer(data, data + size), false);
    }
}

void LibCurlEasyHandle::setRequestDataJsonString(const std::string& jsonString)
{
    mDataPrivate->setPayloadData(std::make_shared<StringPayloadData>(jsonString));
}

void LibCurlEasyHandle::setRequestDataBuffer(ucf::service::network::http::ByteBufferPtr buffer, ucf::service::network::http::UploadProgressFunction progressFunc)
{
    mDataPrivate->setPayloadData(std::make_shared<BufferPayloadData>(buffer, progressFunc));
}

void LibCurlEasyHandle::setRequestDataFile(const std::string& filePath, ucf::service::network::http::UploadProgressFunction progressFunc)
{
    mDataPrivate->setPayloadData(std::make_shared<FilePayloadData>(filePath, progressFunc));
}

size_t LibCurlEasyHandle::readRequestBody(char *data, size_t size)
{
    if (auto payloadData = mDataPrivate->getPayloadData())
    {
        return payloadData->readData(data, size);
    }
    return 0;
}

int LibCurlEasyHandle::seekRequestBody(curl_off_t offset, int origin)
{
    if (auto payloadData = mDataPrivate->getPayloadData())
    {
        return payloadData->seekData(offset, origin);
    }
    return 0;
}

void LibCurlEasyHandle::headersCompleted()
{
    long responseCode{0};
    mDataPrivate->getInfo(CURLINFO_RESPONSE_CODE, &responseCode);
    if (mDataPrivate->getHeaderCallback() != nullptr)
    {
        mDataPrivate->getResponseCode();
        ucf::service::network::http::NetworkHttpResponse response;
        response.setHttpResponseCode(static_cast<int>(responseCode));
        response.setResponseHeaders(mDataPrivate->getResponseHeader());
        mDataPrivate->getHeaderCallback()(response);
    }
}

void LibCurlEasyHandle::finishHandle(CURLcode code)
{
    LIBCURL_LOG_DEBUG("finish rquest, trackingId=" << mDataPrivate->getTrackingId());
    if (code == CURLE_OK)
    {
        if (mDataPrivate->getBodyCallback() != nullptr)
        {
            ucf::service::network::http::ByteBuffer emptyBuf;
            mDataPrivate->getBodyCallback()(emptyBuf, true);
        }
    }
    else
    {
        LIBCURL_LOG_DEBUG("CURL Error (" << static_cast<int>(code) << "): " << curl_easy_strerror(code) << ", trackingId=" << mDataPrivate->getTrackingId());
        //todo
        ucf::service::network::http::NetworkHttpResponse response;

        if (mDataPrivate->getHeaderCallback() != nullptr)
        {
            mDataPrivate->getHeaderCallback()(response);
        }

    }

    if (mDataPrivate->getCompletionCallback() != nullptr)
    {
        mDataPrivate->getCompletionCallback()(mDataPrivate->getResponseMetrics());
    }
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish LibCurlEasyHandle Logic////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}