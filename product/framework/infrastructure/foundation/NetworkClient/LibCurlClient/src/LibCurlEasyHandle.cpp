#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <exception>
#include <curl/curl.h>
#include <ucf/utilities/StringUtils/StringUtils.h>

#include "LibCurlEasyHandle.h"
#include "LibCurlClientLogger.h"
#include "LibCurlPayloadData.h"

namespace ucf::infrastructure::network::libcurl{
inline constexpr auto MAX_REQUEST_REDIRECTS{ 5 };

namespace {

template<typename LogAction>
void logSafely(const LogAction& action) noexcept
{
    try
    {
        action();
    }
    catch (...)
    {
        // Diagnostic failures must not interrupt request completion.
    }
}

void logExceptionSafely(const char* phase, const std::exception_ptr& error) noexcept
{
    if (!error)
    {
        return;
    }
    logSafely([&]() {
        try
        {
            std::rethrow_exception(error);
        }
        catch (const std::exception& ex)
        {
            LIBCURL_LOG_ERROR("Exception during " << phase << ": " << ex.what());
        }
        catch (...)
        {
            LIBCURL_LOG_ERROR("Non-standard exception during " << phase);
        }
    });
}

} // namespace

static size_t header_callback(char *data, size_t size, size_t nmemb, void *userp) noexcept
{
    const auto easyHandle = static_cast<LibCurlEasyHandle*>(userp);
    try
    {
        if(data == nullptr || easyHandle == nullptr)
        {
            return size * nmemb;
        }
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
    catch (...)
    {
        if (easyHandle)
        {
            easyHandle->recordCallbackFailure("response headers");
        }
        return CURL_WRITEFUNC_ERROR;
    }
}

static size_t response_body_callback(char *data, size_t size, size_t nmemb, void *userp) noexcept
{
    const auto easyHandle = static_cast<LibCurlEasyHandle*>(userp);
    try
    {
        if(data == nullptr || easyHandle == nullptr)
        {
            return size * nmemb;
        }
        easyHandle->appendResponseBody(data, size * nmemb);
        return size * nmemb;
    }
    catch (...)
    {
        if (easyHandle)
        {
            easyHandle->recordCallbackFailure("response body");
        }
        return CURL_WRITEFUNC_ERROR;
    }
}

static size_t request_body_callback(char *data, size_t size, size_t nmemb, void *userp) noexcept
{
    const auto easyHandle = static_cast<LibCurlEasyHandle*>(userp);
    try
    {
        if (easyHandle == nullptr)
        {
            LIBCURL_LOG_ERROR(
                "Request-body callback failed: user data is null");
            return 0;
        }
        return easyHandle->readRequestBody(data, size * nmemb);
    }
    catch (...)
    {
        if (easyHandle)
        {
            easyHandle->recordCallbackFailure("request body");
        }
        return CURL_READFUNC_ABORT;
    }
}

int seek_body_callback(void *userp, curl_off_t offset, int origin) noexcept
{
    const auto easyHandle = static_cast<LibCurlEasyHandle*>(userp);
    try
    {
        if (easyHandle == nullptr)
        {
            LIBCURL_LOG_ERROR(
                "Request-body seek callback failed: user data is null");
            return CURL_SEEKFUNC_CANTSEEK;
        }
        return easyHandle->seekRequestBody(offset, origin);
    }
    catch (...)
    {
        if (easyHandle)
        {
            easyHandle->recordCallbackFailure("request body seek");
        }
        return CURL_SEEKFUNC_FAIL;
    }
}

static void logCurlDebugData(curl_infotype type, char *data, size_t size)
{
    if (data == nullptr || type == CURLINFO_SSL_DATA_IN || type == CURLINFO_SSL_DATA_OUT)
    {
        return;
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
            if (!line.empty() && line.back() == '\r')
            {
                line.back() = 0;
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

static int debug_callback(CURL *handle, curl_infotype type, char *data, size_t size, void *userp) noexcept
{
    logSafely([&]() { logCurlDebugData(type, data, size); });
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
    DataPrivate(ucf::infrastructure::network::http::HttpHeaderCallback headerCallback, ucf::infrastructure::network::http::HttpBodyCallback bodyCallback, ucf::infrastructure::network::http::HttpCompletionCallback completionCallback);
    ~DataPrivate();

    CURL* getHandle();

    ucf::infrastructure::network::http::HttpResponseMetrics getResponseMetrics() const;

    const ucf::infrastructure::network::http::HttpHeaderCallback& getHeaderCallback() const {return mHeaderCallback;}
    const ucf::infrastructure::network::http::HttpBodyCallback& getBodyCallback() const {return mBodyCallback;}
    const ucf::infrastructure::network::http::HttpCompletionCallback& getCompletionCallback() const {return mCompletionCallback;}

    void recordCallbackFailure(const char* phase) noexcept
    {
        if (!mCallbackException)
        {
            mCallbackFailurePhase = phase;
            mCallbackException = std::current_exception();
        }
    }
    bool hasCallbackFailure() const noexcept { return static_cast<bool>(mCallbackException); }
    const std::exception_ptr& getCallbackException() const noexcept { return mCallbackException; }
    const char* getCallbackFailurePhase() const noexcept { return mCallbackFailurePhase; }

    void setTrackingId(const std::string& trackingId){mTrackingId = trackingId;}
    std::string getTrackingId() const{return mTrackingId;}
    void setRequestId(const std::string& requestId){mRequestId = requestId;}
    std::string getRequestId() const{return mRequestId;}

    int setHttpMethod(ucf::infrastructure::network::http::HTTPMethod method);
    int setURI(const std::string& uri);
    int setHeaders(const ucf::infrastructure::network::http::NetworkHttpHeaders& headers);

    template<typename ...Args>
    int setOption(CURLoption option, const Args& ...args)
    {
        auto const code = curl_easy_setopt(mHandle, option, args...);
        if (code != CURLE_OK)
        {
            LIBCURL_LOG_ERROR(
                "curl easy option update failed, option: "
                << static_cast<int>(option)
                << ", errorCode: "
                << static_cast<int>(code)
                << ", error: "
                << curl_easy_strerror(code)
                << ", requestId: "
                << mRequestId
                << ", trackingId: "
                << mTrackingId);
        }
        return code;
    }

    template<typename ...Args>
    int getInfo(CURLINFO info, Args ...args) const
    {
        auto const code = curl_easy_getinfo(mHandle, info, args...);
        if (code != CURLE_OK)
        {
            LIBCURL_LOG_WARN(
                "curl easy information query failed, info: "
                << static_cast<int>(info)
                << ", errorCode: "
                << static_cast<int>(code)
                << ", error: "
                << curl_easy_strerror(code)
                << ", requestId: "
                << mRequestId
                << ", trackingId: "
                << mTrackingId);
        }
        return code;
    }
    void addResponseHeader(const std::string& key, const std::string& val);
    ucf::infrastructure::network::http::NetworkHttpHeaders getResponseHeader() const;

    long getResponseCode();

    void setPayloadData(std::shared_ptr<PayloadData> payload);
    std::shared_ptr<PayloadData> getPayloadData() const;
private:
    CURL* mHandle;
    CURLU* mUrl;
    curl_slist* mHeaders;
    ucf::infrastructure::network::http::HttpHeaderCallback mHeaderCallback;
    ucf::infrastructure::network::http::HttpBodyCallback mBodyCallback;
    ucf::infrastructure::network::http::HttpCompletionCallback mCompletionCallback;
    std::string mTrackingId;
    std::string mRequestId;

    ucf::infrastructure::network::http::NetworkHttpHeaders mResponseHeader;
    std::shared_ptr<PayloadData> mPayloadData;
    std::exception_ptr mCallbackException;
    const char* mCallbackFailurePhase{nullptr};
};

LibCurlEasyHandle::DataPrivate::DataPrivate(ucf::infrastructure::network::http::HttpHeaderCallback headerCallback, ucf::infrastructure::network::http::HttpBodyCallback bodyCallback, ucf::infrastructure::network::http::HttpCompletionCallback completionCallback)
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

ucf::infrastructure::network::http::HttpResponseMetrics LibCurlEasyHandle::DataPrivate::getResponseMetrics() const
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

int LibCurlEasyHandle::DataPrivate::setHttpMethod(ucf::infrastructure::network::http::HTTPMethod method)
{
    int result = { 0 };
    switch (method)
    {
        case ucf::infrastructure::network::http::HTTPMethod::GET:
            result = setOption(CURLOPT_HTTPGET, 1L);
            break;
        case ucf::infrastructure::network::http::HTTPMethod::POST:
            setOption(CURLOPT_POST, 1L);
            result = setOption(CURLOPT_POSTFIELDS, nullptr);
            break;
        case ucf::infrastructure::network::http::HTTPMethod::HEAD:
            setOption(CURLOPT_NOBODY, 1L);
            result = setOption(CURLOPT_CUSTOMREQUEST, "HEAD");
            break;
        case ucf::infrastructure::network::http::HTTPMethod::PUT:
            setOption(CURLOPT_CUSTOMREQUEST, "PUT");
            result = setOption(CURLOPT_POSTFIELDS, nullptr);
            break;
        case ucf::infrastructure::network::http::HTTPMethod::DEL:
            result = setOption(CURLOPT_CUSTOMREQUEST, "DELETE");
            break;
        case ucf::infrastructure::network::http::HTTPMethod::PATCH:
            result = setOption(CURLOPT_CUSTOMREQUEST, "PATCH");
            break;
        case ucf::infrastructure::network::http::HTTPMethod::OPTIONS:
            result = setOption(CURLOPT_CUSTOMREQUEST, "OPTIONS");
            break;
        default:
            LIBCURL_LOG_ERROR(
                "HTTP method configuration failed: unsupported method, method: "
                << static_cast<int>(method));
    }
    return result;
}

int LibCurlEasyHandle::DataPrivate::setURI(const std::string& uri)
{
    auto const code = curl_url_set(mUrl, CURLUPART_URL, uri.c_str(), 0);

    if (code != CURLUE_OK)
    {
        LIBCURL_LOG_ERROR(
            "Request URL parsing failed, errorCode: "
            << static_cast<int>(code)
            << ", error: "
            << curl_url_strerror(code));
    }

    setOption(CURLOPT_CURLU, mUrl);

    return code;
}

int LibCurlEasyHandle::DataPrivate::setHeaders(const ucf::infrastructure::network::http::NetworkHttpHeaders& headers)
{
    for (auto[key, val] : headers)
    {
        auto header = key + ": " + val;
        mHeaders = curl_slist_append(mHeaders, header.c_str());
    }
    if (!mHeaders)
    {
        LIBCURL_LOG_ERROR(
            "Request header construction failed, headerCount: "
            << headers.size());
    }
    return setOption(CURLOPT_HTTPHEADER, mHeaders);
}

void LibCurlEasyHandle::DataPrivate::addResponseHeader(const std::string& key, const std::string& val)
{
    mResponseHeader.emplace_back(key, val);
}

ucf::infrastructure::network::http::NetworkHttpHeaders LibCurlEasyHandle::DataPrivate::getResponseHeader() const
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
LibCurlEasyHandle::LibCurlEasyHandle(ucf::infrastructure::network::http::HttpHeaderCallback headerCallback, ucf::infrastructure::network::http::HttpBodyCallback bodyCallback, ucf::infrastructure::network::http::HttpCompletionCallback completionCallback)
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

void LibCurlEasyHandle::setHttpMethod(ucf::infrastructure::network::http::HTTPMethod method)
{
    mDataPrivate->setHttpMethod(method);
}

void LibCurlEasyHandle::setURI(const std::string& uri)
{
    mDataPrivate->setURI(uri);
}

void LibCurlEasyHandle::setHeaders(const ucf::infrastructure::network::http::NetworkHttpHeaders& headers)
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
        mDataPrivate->getBodyCallback()(ucf::infrastructure::network::http::ByteBuffer(data, data + size), false);
    }
}

void LibCurlEasyHandle::setRequestDataString(const std::string& jsonString)
{
    mDataPrivate->setPayloadData(std::make_shared<StringPayloadData>(jsonString));
}

void LibCurlEasyHandle::setRequestDataBuffer(ucf::infrastructure::network::http::ByteBufferPtr buffer, ucf::infrastructure::network::http::UploadProgressFunction progressFunc)
{
    mDataPrivate->setPayloadData(std::make_shared<BufferPayloadData>(buffer, progressFunc));
}

void LibCurlEasyHandle::setRequestDataFile(const std::string& filePath, ucf::infrastructure::network::http::UploadProgressFunction progressFunc)
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

void LibCurlEasyHandle::recordCallbackFailure(const char* phase) noexcept
{
    mDataPrivate->recordCallbackFailure(phase);
}

void LibCurlEasyHandle::finishHandle(CURLcode code) noexcept
{
    if (mFinished.exchange(true, std::memory_order_acq_rel))
    {
        return;
    }

    // Error preparation and business notifications may throw independently.
    const auto notify = [this](const char* phase, const auto& action) noexcept {
        try
        {
            action();
        }
        catch (...)
        {
            recordCallbackFailure(phase);
            logExceptionSafely(phase, std::current_exception());
        }
    };

    logSafely([&]() {
        LIBCURL_LOG_INFO(
            "Network request finished, requestId: "
            << mDataPrivate->getRequestId()
            << ", trackingId: "
            << mDataPrivate->getTrackingId()
            << ", curlCode: "
            << static_cast<int>(code)
            << ", curlError: "
            << curl_easy_strerror(code));
    });
    if (mDataPrivate->hasCallbackFailure())
    {
        logExceptionSafely(mDataPrivate->getCallbackFailurePhase(),
                           mDataPrivate->getCallbackException());
    }

    if (code == CURLE_OK && !mDataPrivate->hasCallbackFailure())
    {
        notify("body completion notification", [&]() {
            if (mDataPrivate->getBodyCallback() != nullptr)
            {
                ucf::infrastructure::network::http::ByteBuffer emptyBuf;
                mDataPrivate->getBodyCallback()(emptyBuf, true);
            }
        });
    }

    if (code != CURLE_OK || mDataPrivate->hasCallbackFailure())
    {
        if (code != CURLE_ABORTED_BY_CALLBACK || mDataPrivate->hasCallbackFailure())
        {
            logSafely([&]() {
                LIBCURL_LOG_WARN(
                    "Network request failed, requestId: "
                    << mDataPrivate->getRequestId()
                    << ", trackingId: "
                    << mDataPrivate->getTrackingId()
                    << ", curlCode: "
                    << static_cast<int>(code)
                    << ", curlError: "
                    << curl_easy_strerror(code)
                    << ", callbackFailure: " << mDataPrivate->hasCallbackFailure());
            });
        }

        notify("error notification", [&]() {
            if (mDataPrivate->getHeaderCallback() != nullptr)
            {
                mDataPrivate->getHeaderCallback()(static_cast<int>(mDataPrivate->getResponseCode()), mDataPrivate->getResponseHeader(), makeErrorData(code));
            }
        });
    }

    if (mDataPrivate->getCompletionCallback() != nullptr)
    {
        ucf::infrastructure::network::http::HttpResponseMetrics metrics;
        try
        {
            metrics = mDataPrivate->getResponseMetrics();
        }
        catch (...)
        {
            logExceptionSafely("response metrics", std::current_exception());
        }
        try
        {
            mDataPrivate->getCompletionCallback()(metrics);
        }
        catch (...)
        {
            // A partially executed completion must never be delivered twice.
            logExceptionSafely("completion notification", std::current_exception());
        }
    }
}

ucf::infrastructure::network::http::ResponseErrorStruct LibCurlEasyHandle::makeErrorData(CURLcode code)
{
    ucf::infrastructure::network::http::ResponseErrorStruct errorStruct;
    errorStruct.errorCode = static_cast<int>(code);
    if (mDataPrivate->hasCallbackFailure())
    {
        errorStruct.errorType = ucf::infrastructure::network::http::ResponseErrorType::OtherError;
        errorStruct.errorDescription = "Local callback failure during ";
        errorStruct.errorDescription += mDataPrivate->getCallbackFailurePhase();
        try
        {
            std::rethrow_exception(mDataPrivate->getCallbackException());
        }
        catch (const std::exception& ex)
        {
            errorStruct.errorDescription += ": ";
            errorStruct.errorDescription += ex.what();
        }
        catch (...)
        {
            errorStruct.errorDescription += ": non-standard exception";
        }
        return errorStruct;
    }
    errorStruct.errorDescription = curl_easy_strerror(code);
    switch (code)
    {
    case CURLE_COULDNT_RESOLVE_HOST:
    case CURLE_COULDNT_RESOLVE_PROXY:
        errorStruct.errorType = ucf::infrastructure::network::http::ResponseErrorType::DNSError;
        break;

    case CURLE_COULDNT_CONNECT:
    case CURLE_SEND_ERROR:
    case CURLE_RECV_ERROR:
        errorStruct.errorType = ucf::infrastructure::network::http::ResponseErrorType::SocketError;
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
        errorStruct.errorType = ucf::infrastructure::network::http::ResponseErrorType::TLSError;
        break;
    case CURLE_OPERATION_TIMEDOUT:
        errorStruct.errorType = ucf::infrastructure::network::http::ResponseErrorType::TimeoutError;
        break;
    case CURLE_ABORTED_BY_CALLBACK:
        errorStruct.errorType = ucf::infrastructure::network::http::ResponseErrorType::CanceledError;
        break;
    case CURLE_REMOTE_ACCESS_DENIED:
        errorStruct.errorType = ucf::infrastructure::network::http::ResponseErrorType::OtherError;
        break;
    default:
        errorStruct.errorType = ucf::infrastructure::network::http::ResponseErrorType::UnHandledError;
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
