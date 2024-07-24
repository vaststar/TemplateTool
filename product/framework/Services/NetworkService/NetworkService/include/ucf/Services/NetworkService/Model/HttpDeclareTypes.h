#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <optional>
#include <ucf/Services/ServiceCommonFile/ServiceExport.h>

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
    CancelledError,
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
}