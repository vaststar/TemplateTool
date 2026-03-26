#pragma once

#include <ucf/Services/NetworkService/Model/HttpDeclareTypes.h>
#include <ucf/Agents/NetworkAgent/NetworkModelTypes/Http/NetworkHttpTypes.h>

namespace ucf::service::network::http {

/// @brief Maximum number of HTTP redirects to follow
constexpr int kMaxRedirectCount = 3;

/// @brief Convert service layer HTTPMethod to utilities layer HTTPMethod
inline ucf::agents::network::http::HTTPMethod 
convertToUtilitiesHttpMethod(HTTPMethod method)
{
    switch (method)
    {
    case HTTPMethod::GET:     return ucf::agents::network::http::HTTPMethod::GET;
    case HTTPMethod::POST:    return ucf::agents::network::http::HTTPMethod::POST;
    case HTTPMethod::HEAD:    return ucf::agents::network::http::HTTPMethod::HEAD;
    case HTTPMethod::PUT:     return ucf::agents::network::http::HTTPMethod::PUT;
    case HTTPMethod::DEL:     return ucf::agents::network::http::HTTPMethod::DEL;
    case HTTPMethod::PATCH:   return ucf::agents::network::http::HTTPMethod::PATCH;
    case HTTPMethod::OPTIONS: return ucf::agents::network::http::HTTPMethod::OPTIONS;
    default:                  return ucf::agents::network::http::HTTPMethod::GET;
    }
}

/// @brief Convert utilities layer ResponseErrorStruct to service layer ResponseErrorStruct
inline ResponseErrorStruct 
convertToServiceErrorStruct(const ucf::agents::network::http::ResponseErrorStruct& error)
{
    ResponseErrorStruct result;
    result.errorCode = error.errorCode;
    result.errorDescription = error.errorDescription;
    
    switch (error.errorType)
    {
    case ucf::agents::network::http::ResponseErrorType::NoError:
        result.errorType = ResponseErrorType::NoError;
        break;
    case ucf::agents::network::http::ResponseErrorType::DNSError:
        result.errorType = ResponseErrorType::DNSError;
        break;
    case ucf::agents::network::http::ResponseErrorType::SocketError:
        result.errorType = ResponseErrorType::SocketError;
        break;
    case ucf::agents::network::http::ResponseErrorType::TLSError:
        result.errorType = ResponseErrorType::TLSError;
        break;
    case ucf::agents::network::http::ResponseErrorType::TimeoutError:
        result.errorType = ResponseErrorType::TimeoutError;
        break;
    case ucf::agents::network::http::ResponseErrorType::CanceledError:
        result.errorType = ResponseErrorType::CanceledError;
        break;
    case ucf::agents::network::http::ResponseErrorType::OtherError:
        result.errorType = ResponseErrorType::OtherError;
        break;
    case ucf::agents::network::http::ResponseErrorType::UnHandledError:
        result.errorType = ResponseErrorType::UnHandledError;
        break;
    default:
        result.errorType = ResponseErrorType::UnHandledError;
        break;
    }
    return result;
}

} // namespace ucf::service::network::http
