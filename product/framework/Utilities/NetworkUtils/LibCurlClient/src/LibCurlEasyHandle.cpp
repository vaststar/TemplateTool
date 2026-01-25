#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <curl/curl.h>
#include <ucf/Utilities/StringUtils/StringUtils.h>

#include "LibCurlEasyHandle.h"
#include "LibCurlClientLogger.h"
#include "LibCurlPayloadData.h"

namespace ucf::utilities::network::libcurl{
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
    return 0;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class LibCurlEasyHandle::DataPrivate
{
public:
    DataPrivate(ucf::utilities::network::http::HttpHeaderCallback headerCallback, ucf::utilities::network::http::HttpBodyCallback bodyCallback, ucf::utilities::network::http::HttpCompletionCallback completionCallback);
    ~DataPrivate();

    CURL* getHandle();

    ucf::utilities::network::http::HttpResponseMetrics getResponseMetrics() const;

    ucf::utilities::network::http::HttpHeaderCallback getHeaderCallback() {return mHeaderCallback;}
    ucf::utilities::network::http::HttpBodyCallback getBodyCallback() {return mBodyCallback;}
    ucf::utilities::network::http::HttpCompletionCallback getCompletionCallback() {return mCompletionCallback;}

    void setTrackingId(const std::string& trackingId){mTrackingId = trackingId;}
    std::string getTrackingId() const{return mTrackingId;}
    void setRequestId(const std::string& requestId){mRequestId = requestId;}
    std::string getRequestId() const{return mRequestId;}

    int setHttpMethod(ucf::utilities::network::http::HTTPMethod method);
    int setURI(const std::string& uri);
    int setHeaders(const ucf::utilities::network::http::NetworkHttpHeaders& headers);

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
    ucf::utilities::network::http::NetworkHttpHeaders getResponseHeader() const;

    long getResponseCode();

    void setPayloadData(std::shared_ptr<PayloadData> payload);
    std::shared_ptr<PayloadData> getPayloadData() const;
private:
    CURL* mHandle;
    CURLU* mUrl;
    curl_slist* mHeaders;
    ucf::utilities::network::http::HttpHeaderCallback mHeaderCallback;
    ucf::utilities::network::http::HttpBodyCallback mBodyCallback;
    ucf::utilities::network::http::HttpCompletionCallback mCompletionCallback;
    std::string mTrackingId;
    std::string mRequestId;

    ucf::utilities::network::http::NetworkHttpHeaders mResponseHeader;
    std::shared_ptr<PayloadData> mPayloadData;
};

LibCurlEasyHandle::DataPrivate::DataPrivate(ucf::utilities::network::http::HttpHeaderCallback headerCallback, ucf::utilities::network::http::HttpBodyCallback bodyCallback, ucf::utilities::network::http::HttpCompletionCallback completionCallback)
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

ucf::utilities::network::http::HttpResponseMetrics LibCurlEasyHandle::DataPrivate::getResponseMetrics() const
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

int LibCurlEasyHandle::DataPrivate::setHttpMethod(ucf::utilities::network::http::HTTPMethod method)
{
    int result = { 0 };
    switch (method)
    {
        case ucf::utilities::network::http::HTTPMethod::GET:
            result = setOption(CURLOPT_HTTPGET, 1L);
            break;
        case ucf::utilities::network::http::HTTPMethod::POST:
            setOption(CURLOPT_POST, 1L);
            result = setOption(CURLOPT_POSTFIELDS, nullptr);
            break;
        case ucf::utilities::network::http::HTTPMethod::HEAD:
            setOption(CURLOPT_NOBODY, 1L);
            result = setOption(CURLOPT_CUSTOMREQUEST, "HEAD");
            break;
        case ucf::utilities::network::http::HTTPMethod::PUT:
            setOption(CURLOPT_CUSTOMREQUEST, "PUT");
            result = setOption(CURLOPT_POSTFIELDS, nullptr);
            break;
        case ucf::utilities::network::http::HTTPMethod::DEL:
            result = setOption(CURLOPT_CUSTOMREQUEST, "DELETE");
            break;
        case ucf::utilities::network::http::HTTPMethod::PATCH:
            result = setOption(CURLOPT_CUSTOMREQUEST, "PATCH");
            break;
        case ucf::utilities::network::http::HTTPMethod::OPTIONS:
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

int LibCurlEasyHandle::DataPrivate::setHeaders(const ucf::utilities::network::http::NetworkHttpHeaders& headers)
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

ucf::utilities::network::http::NetworkHttpHeaders LibCurlEasyHandle::DataPrivate::getResponseHeader() const
{
    return mResponseHeader;
}

long LibCurlEasyHandle::DataPrivate::getResponseCode()
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
LibCurlEasyHandle::LibCurlEasyHandle(ucf::utilities::network::http::HttpHeaderCallback headerCallback, ucf::utilities::network::http::HttpBodyCallback bodyCallback, ucf::utilities::network::http::HttpCompletionCallback completionCallback)
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

void LibCurlEasyHandle::setHttpMethod(ucf::utilities::network::http::HTTPMethod method)
{
    mDataPrivate->setHttpMethod(method);
}

void LibCurlEasyHandle::setURI(const std::string& uri)
{
    mDataPrivate->setURI(uri);
}

void LibCurlEasyHandle::setHeaders(const ucf::utilities::network::http::NetworkHttpHeaders& headers)
{
    mDataPrivate->setHeaders(headers);
}

void LibCurlEasyHandle::setTrackingId(const std::string& trackingId)
{
    mDataPrivate->setTrackingId(trackingId);
}

void LibCurlEasyHandle::setRequestId(const std::string& requestId)
{
    mDataPrivate->setRequestId(requestId);
}

std::string LibCurlEasyHandle::getRequestId() const
{
    return mDataPrivate->getRequestId();
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

    mDataPrivate->setOption(CURLOPT_NOSIGNAL, 1L);
    
    // Connection timeout (TCP + TLS handshake), default 30 seconds
    mDataPrivate->setOption(CURLOPT_CONNECTTIMEOUT, 30L);
    
    // Low speed detection: timeout if speed < 1KB/s for 30 seconds
    mDataPrivate->setOption(CURLOPT_LOW_SPEED_LIMIT, 1000L);
    mDataPrivate->setOption(CURLOPT_LOW_SPEED_TIME, 30L);
}

void LibCurlEasyHandle::enableCURLDebugPrint()
{
    mDataPrivate->setOption(CURLOPT_VERBOSE, 1L);
    mDataPrivate->setOption(CURLOPT_DEBUGFUNCTION, debug_callback);
}

void LibCurlEasyHandle::addResponseHeader(const std::string& key, const std::string& val)
{
    mDataPrivate->addResponseHeader(key, val);
}

void LibCurlEasyHandle::appendResponseBody(char *data, size_t size)
{
    if (mDataPrivate->getBodyCallback() != nullptr)
    {
        mDataPrivate->getBodyCallback()(ucf::utilities::network::http::ByteBuffer(data, data + size), false);
    }
}

void LibCurlEasyHandle::setRequestDataString(const std::string& jsonString)
{
    mDataPrivate->setPayloadData(std::make_shared<StringPayloadData>(jsonString));
}

void LibCurlEasyHandle::setRequestDataBuffer(ucf::utilities::network::http::ByteBufferPtr buffer, ucf::utilities::network::http::UploadProgressFunction progressFunc)
{
    mDataPrivate->setPayloadData(std::make_shared<BufferPayloadData>(buffer, progressFunc));
}

void LibCurlEasyHandle::setRequestDataFile(const std::string& filePath, ucf::utilities::network::http::UploadProgressFunction progressFunc)
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
    if (mDataPrivate->getHeaderCallback() != nullptr)
    {
        mDataPrivate->getHeaderCallback()(static_cast<int>(mDataPrivate->getResponseCode()), mDataPrivate->getResponseHeader(), std::nullopt);
    }
}

void LibCurlEasyHandle::finishHandle(CURLcode code)
{
    LIBCURL_LOG_INFO("[REQUEST_FINISH] requestId=" << mDataPrivate->getRequestId() 
                     << ", trackingId=" << mDataPrivate->getTrackingId() 
                     << ", code=" << static_cast<int>(code) 
                     << " (" << curl_easy_strerror(code) << ")");
    if (code == CURLE_OK)
    {
        if (mDataPrivate->getBodyCallback() != nullptr)
        {
            ucf::utilities::network::http::ByteBuffer emptyBuf;
            mDataPrivate->getBodyCallback()(emptyBuf, true);
        }
    }
    else
    {
        LIBCURL_LOG_DEBUG("CURL Error (" << static_cast<int>(code) << "): " << curl_easy_strerror(code) << ", trackingId=" << mDataPrivate->getTrackingId());
        if (mDataPrivate->getHeaderCallback() != nullptr)
        {
            mDataPrivate->getHeaderCallback()(static_cast<int>(mDataPrivate->getResponseCode()), mDataPrivate->getResponseHeader(), makeErrorData(code));
        }

    }

    if (mDataPrivate->getCompletionCallback() != nullptr)
    {
        mDataPrivate->getCompletionCallback()(mDataPrivate->getResponseMetrics());
    }
}

ucf::utilities::network::http::ResponseErrorStruct LibCurlEasyHandle::makeErrorData(CURLcode code)
{
    ucf::utilities::network::http::ResponseErrorStruct errorStruct;
    errorStruct.errorDescription = curl_easy_strerror(code);
    errorStruct.errorCode = static_cast<int>(code);
    switch (code)
    {
    case CURLE_COULDNT_RESOLVE_HOST:
    case CURLE_COULDNT_RESOLVE_PROXY:
        errorStruct.errorType = ucf::utilities::network::http::ResponseErrorType::DNSError;
        break;
    
    case CURLE_COULDNT_CONNECT:
    case CURLE_SEND_ERROR:
    case CURLE_RECV_ERROR:
        errorStruct.errorType = ucf::utilities::network::http::ResponseErrorType::SocketError;
        break;
    case CURLE_SSL_ENGINE_NOTFOUND:
    case CURLE_SSL_ENGINE_SETFAILED:
    case CURLE_SSL_CERTPROBLEM:
    case CURLE_SSL_CIPHER:
    case CURLE_SSL_ENGINE_INITFAILED:
    case CURLE_SSL_CACERT_BADFILE:
    case CURLE_SSL_SHUTDOWN_FAILED:
    case CURLE_SSL_CRL_BADFILE:
    case CURLE_SSL_ISSUER_ERROR:
    case CURLE_SSL_PINNEDPUBKEYNOTMATCH:
    case CURLE_SSL_INVALIDCERTSTATUS:
    case CURLE_SSL_CLIENTCERT:
    case CURLE_SSL_CONNECT_ERROR:
    case CURLE_PEER_FAILED_VERIFICATION:
        errorStruct.errorType = ucf::utilities::network::http::ResponseErrorType::TLSError;
        break;
    case CURLE_OPERATION_TIMEDOUT:
        errorStruct.errorType = ucf::utilities::network::http::ResponseErrorType::TimeoutError;
        break;
    case CURLE_REMOTE_ACCESS_DENIED:
    case CURLE_ABORTED_BY_CALLBACK:
        errorStruct.errorType = ucf::utilities::network::http::ResponseErrorType::OtherError;
        break;
    default:
        errorStruct.errorType = ucf::utilities::network::http::ResponseErrorType::UnHandledError;
        break;
    }
    return errorStruct;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish LibCurlEasyHandle Logic////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}