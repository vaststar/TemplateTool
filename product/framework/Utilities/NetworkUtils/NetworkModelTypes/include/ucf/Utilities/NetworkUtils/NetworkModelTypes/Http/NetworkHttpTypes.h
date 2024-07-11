#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <optional>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/NetworkModelTypesExport.h>

namespace ucf::utilities::network::http{

enum class HTTPMethod{
    GET,
    POST,
    HEAD,
    PUT,
    DEL,
    PATCH,
    OPTIONS
};

enum class NetworkHttpPayloadType{
    None,
    Json,
    File,
    Memory
};

enum class ResponseErrorType
{
    NoError,
    DNSError,
    SocketError,
    TLSError,
    TimeoutError,
    CancelledError,
    OtherError,
    UnHandledError
};

struct NETWORKTYPE_EXPORT ResponseErrorStruct{
    int errorCode{ 0 };
    ResponseErrorType errorType{ResponseErrorType::NoError};
    std::string errorDescription;
};

struct NETWORKTYPE_EXPORT HttpResponseMetrics{
    int requestStartTime{ 0 };
    int dnsLookupMs{ 0 };
    int connectMs{ 0 };
    int sendRequestMs{ 0 };
    int receiveResponseMs{ 0 };
    size_t responseLength{ 0 };
    std::string httpVersion;
};

using ByteBuffer = std::vector<uint8_t>;
using ByteBufferPtr = std::shared_ptr<ByteBuffer>;
using NetworkHttpHeaders = std::vector<std::pair<std::string, std::string>>;
using HttpHeaderCallback = std::function<void(int statusCode, const NetworkHttpHeaders& headers, std::optional<ResponseErrorStruct> errorData)>;
using HttpBodyCallback =  std::function<void(const ByteBuffer& buffer, bool isFinished)>;
using HttpCompletionCallback = std::function<void(const HttpResponseMetrics& metrics)>;
using UploadProgressFunction = std::function<void(size_t, uint64_t)>;
}