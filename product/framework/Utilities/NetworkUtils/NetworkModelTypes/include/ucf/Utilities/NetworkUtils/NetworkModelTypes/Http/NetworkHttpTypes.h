#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <optional>
#include <cstdint>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/NetworkModelTypesExport.h>

namespace ucf::utilities::network::http{

/**
 * @brief HTTP request methods.
 * @see https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods
 */
enum class NETWORKTYPE_EXPORT HTTPMethod{
    GET,        ///< Retrieve data from the server
    POST,       ///< Submit data to the server
    HEAD,       ///< Same as GET but without response body
    PUT,        ///< Replace the target resource
    DEL,        ///< Delete the specified resource (named DEL to avoid C++ keyword conflict)
    PATCH,      ///< Partial modification of a resource
    OPTIONS,    ///< Describe communication options for the target resource
    Unknown     ///< Unknown or uninitialized method
};

/**
 * @brief Type of HTTP request payload/body.
 */
enum class NetworkHttpPayloadType{
    None,       ///< No payload
    String,     ///< Payload is a string (e.g., JSON, XML)
    File,       ///< Payload is read from a file path
    Memory      ///< Payload is a raw memory buffer
};

/**
 * @brief Categories of HTTP response errors.
 */
enum class ResponseErrorType
{
    NoError,        ///< Request completed successfully
    DNSError,       ///< DNS resolution failed
    SocketError,    ///< TCP connection failed
    TLSError,       ///< SSL/TLS handshake or certificate error
    TimeoutError,   ///< Request timed out (connect or transfer)
    CanceledError,  ///< Request was cancelled by user
    OtherError,     ///< Other known error
    UnHandledError  ///< Unexpected/unhandled error
};

/**
 * @brief Detailed error information for failed HTTP requests.
 */
struct NETWORKTYPE_EXPORT ResponseErrorStruct{
    int errorCode{ 0 };                                     ///< Native error code (e.g., CURLcode)
    ResponseErrorType errorType{ResponseErrorType::NoError}; ///< Error category
    std::string errorDescription;                            ///< Human-readable error message
};

/**
 * @brief Performance metrics for HTTP request/response cycle.
 * @note All time values are in milliseconds unless otherwise specified.
 */
struct NETWORKTYPE_EXPORT HttpResponseMetrics{
    int64_t requestStartTime{ 0 };  ///< Unix timestamp (ms) when request started
    int dnsLookupMs{ 0 };           ///< Time spent on DNS lookup
    int connectMs{ 0 };             ///< Time spent on TCP + TLS connection
    int sendRequestMs{ 0 };         ///< Time spent sending request
    int receiveResponseMs{ 0 };     ///< Time spent receiving response
    size_t responseLength{ 0 };     ///< Total response body size in bytes
    std::string httpVersion;        ///< HTTP version used (e.g., "HTTP/1.1", "HTTP/2")
};

/// @brief Raw byte buffer for binary data.
using ByteBuffer = std::vector<uint8_t>;
/// @brief Shared pointer to a byte buffer.
using ByteBufferPtr = std::shared_ptr<ByteBuffer>;
/// @brief HTTP headers as key-value pairs (allows duplicate keys).
using NetworkHttpHeaders = std::vector<std::pair<std::string, std::string>>;

/**
 * @brief Callback invoked when HTTP response headers are received.
 * @param statusCode HTTP status code (e.g., 200, 404)
 * @param headers Response headers
 * @param errorData Error information if request failed before headers
 */
using HttpHeaderCallback = std::function<void(int statusCode, const NetworkHttpHeaders& headers, std::optional<ResponseErrorStruct> errorData)>;

/**
 * @brief Callback invoked as response body data arrives.
 * @param buffer Chunk of response body data
 * @param isFinished True if this is the final chunk
 */
using HttpBodyCallback = std::function<void(const ByteBuffer& buffer, bool isFinished)>;

/**
 * @brief Callback invoked when the entire request/response cycle completes.
 * @param metrics Performance metrics for the completed request
 */
using HttpCompletionCallback = std::function<void(const HttpResponseMetrics& metrics)>;

/**
 * @brief Callback for upload progress reporting.
 * @param bytesSent Number of bytes uploaded so far
 * @param totalBytes Total bytes to upload (0 if unknown)
 */
using UploadProgressFunction = std::function<void(size_t bytesSent, uint64_t totalBytes)>;

}