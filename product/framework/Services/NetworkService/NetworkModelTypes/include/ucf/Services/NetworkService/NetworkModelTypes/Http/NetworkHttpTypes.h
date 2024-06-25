#pragma once
#include <string>
#include <vector>
#include <functional>
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

using NetworkHttpHeaders = std::vector<std::pair<std::string, std::string>>;
class NetworkHttpRequest;
class NetworkHttpResponse;
struct HttpResponseMetrics;
using ByteBuffer = std::vector<uint8_t>;
using HttpHeaderCallback = std::function<void(const NetworkHttpResponse& response)>;
using HttpBodyCallback =  std::function<void(const ByteBuffer& buffer, bool isFinished)>;
using HttpCompletionCallback = std::function<void(const HttpResponseMetrics& metrics)>;
}