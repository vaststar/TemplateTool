#pragma once

#include <ucf/Services/NetworkService/Model/HttpDeclareTypes.h>
#include <ucf/Infrastructure/NetworkClient/NetworkModelTypes/Http/NetworkHttpTypes.h>

namespace ucf::service::network::http {

/// @brief Maximum number of HTTP redirects to follow
constexpr int kMaxRedirectCount = 3;

/// @brief Convert service layer HTTPMethod to utilities layer HTTPMethod
inline ucf::infrastructure::network::http::HTTPMethod 
convertToUtilitiesHttpMethod(HTTPMethod method)
{
    switch (method)
    {
    case HTTPMethod::GET:     return ucf::infrastructure::network::http::HTTPMethod::GET;
    case HTTPMethod::POST:    return ucf::infrastructure::network::http::HTTPMethod::POST;
    case HTTPMethod::HEAD:    return ucf::infrastructure::network::http::HTTPMethod::HEAD;
    case HTTPMethod::PUT:     return ucf::infrastructure::network::http::HTTPMethod::PUT;
    case HTTPMethod::DEL:     return ucf::infrastructure::network::http::HTTPMethod::DEL;
    case HTTPMethod::PATCH:   return ucf::infrastructure::network::http::HTTPMethod::PATCH;
    case HTTPMethod::OPTIONS: return ucf::infrastructure::network::http::HTTPMethod::OPTIONS;
    default:                  return ucf::infrastructure::network::http::HTTPMethod::GET;
    }
}

/// @brief Convert utilities layer ResponseErrorStruct to service layer ResponseErrorStruct
inline ResponseErrorStruct 
convertToServiceErrorStruct(const ucf::infrastructure::network::http::ResponseErrorStruct& error)
{
    ResponseErrorStruct result;
    result.errorCode = error.errorCode;
    result.errorDescription = error.errorDescription;
    
    switch (error.errorType)
    {
    case ucf::infrastructure::network::http::ResponseErrorType::NoError:
        result.errorType = ResponseErrorType::NoError;
        break;
    case ucf::infrastructure::network::http::ResponseErrorType::DNSError:
        result.errorType = ResponseErrorType::DNSError;
        break;
    case ucf::infrastructure::network::http::ResponseErrorType::SocketError:
        result.errorType = ResponseErrorType::SocketError;
        break;
    case ucf::infrastructure::network::http::ResponseErrorType::TLSError:
        result.errorType = ResponseErrorType::TLSError;
        break;
    case ucf::infrastructure::network::http::ResponseErrorType::TimeoutError:
        result.errorType = ResponseErrorType::TimeoutError;
        break;
    case ucf::infrastructure::network::http::ResponseErrorType::CanceledError:
        result.errorType = ResponseErrorType::CanceledError;
        break;
    case ucf::infrastructure::network::http::ResponseErrorType::OtherError:
        result.errorType = ResponseErrorType::OtherError;
        break;
    case ucf::infrastructure::network::http::ResponseErrorType::UnHandledError:
        result.errorType = ResponseErrorType::UnHandledError;
        break;
    default:
        result.errorType = ResponseErrorType::UnHandledError;
        break;
    }
    return result;
}

} // namespace ucf::service::network::http
