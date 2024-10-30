#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <optional>
#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

namespace ucf::service::network::http{

enum class SERVICE_EXPORT HTTPMethod{
    GET,
    POST,
    HEAD,
    PUT,
    DEL,
    PATCH,
    OPTIONS,
    Unknwon
};

enum class SERVICE_EXPORT ResponseErrorType
{
    NoError,
    DNSError,
    SocketError,
    TLSError,
    TimeoutError,
    CanceledError,
    OtherError,
    UnHandledError
};

struct SERVICE_EXPORT ResponseErrorStruct{
    int errorCode{ 0 };
    ResponseErrorType errorType{ResponseErrorType::NoError};
    std::string errorDescription;
};

using ByteBuffer = std::vector<uint8_t>;
using ByteBufferPtr = std::shared_ptr<ByteBuffer>;
using NetworkHttpHeaders = std::vector<std::pair<std::string, std::string>>;
using HttpHeaderCallback = std::function<void(int statusCode, const NetworkHttpHeaders& headers, std::optional<ResponseErrorStruct> errorData)>;
using HttpBodyCallback =  std::function<void(const ByteBuffer& buffer, bool isFinished)>;
using UploadProgressFunction = std::function<void(size_t, uint64_t)>;


class HttpRestResponse;
using HttpRestResponseCallbackFunc = std::function<void(const ucf::service::network::http::HttpRestResponse&)>;

class HttpRawResponse;
using HttpRawResponseCallbackFunc = std::function<void(const ucf::service::network::http::HttpRawResponse&)>;

class HttpDownloadToMemoryResponse;
using HttpDownloadToMemoryResponseCallbackFunc = std::function<void(const ucf::service::network::http::HttpDownloadToMemoryResponse&)>;

class HttpDownloadToFileResponse;
using HttpDownloadToFileResponseCallbackFunc = std::function<void(const ucf::service::network::http::HttpDownloadToFileResponse&)>;

}