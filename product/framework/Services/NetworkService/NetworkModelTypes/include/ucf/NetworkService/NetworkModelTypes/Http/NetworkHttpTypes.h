#pragma once

namespace ucf::network::http{
enum class HTTPMethod{
    GET,
    POST
};

class NetworkHttpResponse;
struct HttpResponseMetrics;
using ByteBuffer = std::vector<uint8_t>;
using HttpHeaderCallback = std::function<void(const NetworkHttpResponse& response)>;
using HttpBodyCallback =  std::function<void(const ByteBuffer& buffer, bool isFinished)>;
using HttpCompletionCallback = std::function<void HttpResponseMetrics& metrics>;
}