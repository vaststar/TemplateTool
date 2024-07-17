#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <optional>

namespace ucf::service::network::http{
enum class HTTPMethod{
    GET,
    POST,
    HEAD,
    PUT,
    DEL,
    PATCH,
    OPTIONS
};
class HttpRestResponse;
class HttpRestRequest;
using HttpRestResponseCallbackFunc = std::function<void(const ucf::service::network::http::HttpRestResponse& restResponse)>;
using HttpRequestHeaders = std::vector<std::pair<std::string, std::string>>;
}