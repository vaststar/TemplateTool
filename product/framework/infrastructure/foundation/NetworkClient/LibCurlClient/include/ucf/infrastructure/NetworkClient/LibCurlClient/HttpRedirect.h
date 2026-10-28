#pragma once

#include <ucf/infrastructure/NetworkClient/LibCurlClient/LibCurlClientExport.h>
#include <ucf/infrastructure/NetworkClient/NetworkModelTypes/http/NetworkHttpTypes.h>

namespace ucf::infrastructure::network::http {
class NetworkHttpRequest;
}

namespace ucf::infrastructure::network::libcurl {

// Prepare the next HTTP(S) request for a 301/302/303/307/308 response.
// A missing or invalid Location returns false without modifying the request.
// 301/302 change POST to GET; 303 keeps HEAD or uses GET and drops the payload.
// Cross-origin redirects remove Authorization, Cookie, Proxy-Authorization,
// Host and URL credentials. Other custom authentication headers are preserved.
// Request/tracking IDs and timeout are preserved. Exceptions propagate.
LIB_CURL_CLIENT_API bool tryApplyHttpRedirect(
    http::NetworkHttpRequest& request,
    int statusCode,
    const http::NetworkHttpHeaders& responseHeaders);

} // namespace ucf::infrastructure::network::libcurl
