#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <catch2/catch_test_macros.hpp>
#include <curl/curl.h>
#include <openssl/pem.h>
#include <openssl/x509v3.h>

#include <ucf/infrastructure/NetworkClient/LibCurlClient/HttpRedirect.h>
#include <ucf/infrastructure/NetworkClient/NetworkModelTypes/http/NetworkHttpRequest.h>

#include "LibCurlEasyHandle.h"

namespace {

using ucf::infrastructure::network::http::NetworkHttpHeaders;
using ucf::infrastructure::network::http::NetworkHttpRequest;
using ucf::infrastructure::network::libcurl::LibCurlEasyHandle;
using ucf::infrastructure::network::libcurl::tryApplyHttpRedirect;

using Key = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
using Certificate = std::unique_ptr<X509, decltype(&X509_free)>;
using asio::ip::tcp;

Key makeKey()
{
    Key key(EVP_PKEY_Q_keygen(nullptr, nullptr, "EC", "prime256v1"), &EVP_PKEY_free);
    REQUIRE(key);
    return key;
}

void addExtension(X509* certificate, int type, const char* value)
{
    X509V3_CTX context{};
    X509V3_set_ctx(&context, nullptr, certificate, nullptr, nullptr, 0);
    std::unique_ptr<X509_EXTENSION, decltype(&X509_EXTENSION_free)> extension(
        X509V3_EXT_nconf_nid(nullptr, &context, type, value), &X509_EXTENSION_free);
    REQUIRE(extension);
    REQUIRE(X509_add_ext(certificate, extension.get(), -1) == 1);
}

Certificate makeCertificate(EVP_PKEY* key, const char* name, long serial,
                            X509* issuer, EVP_PKEY* issuerKey)
{
    Certificate certificate(X509_new(), &X509_free);
    REQUIRE(certificate);
    REQUIRE(X509_set_version(certificate.get(), 2) == 1);
    REQUIRE(ASN1_INTEGER_set(X509_get_serialNumber(certificate.get()), serial) == 1);
    REQUIRE(X509_gmtime_adj(X509_getm_notBefore(certificate.get()), -60));
    REQUIRE(X509_gmtime_adj(X509_getm_notAfter(certificate.get()), 3600));
    REQUIRE(X509_set_pubkey(certificate.get(), key) == 1);

    auto* subject = X509_get_subject_name(certificate.get());
    REQUIRE(X509_NAME_add_entry_by_txt(subject, "CN", MBSTRING_ASC,
        reinterpret_cast<const unsigned char*>(name), -1, -1, 0) == 1);
    REQUIRE(X509_set_issuer_name(certificate.get(), issuer ? X509_get_subject_name(issuer) : subject) == 1);
    if (issuer)
    {
        addExtension(certificate.get(), NID_basic_constraints, "critical,CA:FALSE");
        addExtension(certificate.get(), NID_ext_key_usage, "serverAuth");
        addExtension(certificate.get(), NID_subject_alt_name, "DNS:localhost");
    }
    else
    {
        addExtension(certificate.get(), NID_basic_constraints, "critical,CA:TRUE");
        addExtension(certificate.get(), NID_key_usage, "critical,keyCertSign");
    }
    REQUIRE(X509_sign(certificate.get(), issuerKey, EVP_sha256()) > 0);
    return certificate;
}

std::string toPem(X509* certificate)
{
    std::unique_ptr<BIO, decltype(&BIO_free)> output(BIO_new(BIO_s_mem()), &BIO_free);
    REQUIRE(output);
    REQUIRE(PEM_write_bio_X509(output.get(), certificate) == 1);
    char* data = nullptr;
    const auto size = BIO_get_mem_data(output.get(), &data);
    REQUIRE(size > 0);
    return std::string(data, static_cast<std::size_t>(size));
}

class CurlRuntime
{
public:
    CurlRuntime()
    {
        if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK)
        {
            throw std::runtime_error("curl_global_init failed");
        }
    }

    ~CurlRuntime() { curl_global_cleanup(); }

    CurlRuntime(const CurlRuntime&) = delete;
    CurlRuntime& operator=(const CurlRuntime&) = delete;
};

class LoopbackTlsServer
{
public:
    LoopbackTlsServer(X509* certificate, EVP_PKEY* key, std::string_view response)
        : mContext(asio::ssl::context::tls_server)
        , mAcceptor(mIo, {asio::ip::address_v4::loopback(), 0})
        , mStream(mIo, mContext)
        , mPort(mAcceptor.local_endpoint().port())
        , mResponse(response)
    {
        REQUIRE(SSL_use_certificate(mStream.native_handle(), certificate) == 1);
        REQUIRE(SSL_use_PrivateKey(mStream.native_handle(), key) == 1);
        mAcceptor.async_accept(mStream.lowest_layer(), [this](const asio::error_code& error) {
            if (!error)
            {
                mStream.async_handshake(asio::ssl::stream_base::server, [this](const asio::error_code& handshakeError) {
                    if (!handshakeError)
                    {
                        readRequest();
                    }
                });
            }
        });
        mThread = std::jthread([this] { mIo.run(); });
    }

    ~LoopbackTlsServer() { mIo.stop(); }

    unsigned short port() const { return mPort; }

private:
    void readRequest()
    {
        asio::async_read_until(mStream, asio::dynamic_buffer(mRequest, 4096), "\r\n\r\n",
            [this](const asio::error_code& error, std::size_t) {
                if (!error)
                {
                    asio::async_write(mStream, asio::buffer(mResponse),
                        [this](const asio::error_code& writeError, std::size_t) {
                            if (!writeError)
                            {
                                mStream.async_shutdown([](const asio::error_code&) {});
                            }
                        });
                }
            });
    }

    asio::io_context mIo;
    asio::ssl::context mContext;
    tcp::acceptor mAcceptor;
    asio::ssl::stream<tcp::socket> mStream;
    unsigned short mPort;
    std::string mRequest;
    std::string mResponse;
    std::jthread mThread;
};

} // namespace

TEST_CASE("Network TLS verifies the certificate chain and host", "[tls]")
{
    CurlRuntime runtime;
    auto rootKey = makeKey();
    auto root = makeCertificate(rootKey.get(), "TemplateTool test root", 1, nullptr, rootKey.get());
    auto serverKey = makeKey();
    auto serverCertificate = makeCertificate(serverKey.get(), "localhost", 2, root.get(), rootKey.get());
    auto pem = toPem(root.get());
    std::string host = "localhost";
    std::string_view response = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\nConnection: close\r\n\r\nok";
    auto expected = CURLE_OK;
    int expectedStatus = 200;
    NetworkHttpHeaders expectedHeaders{{"Content-Length", "2"}, {"Connection", "close"}};
    bool rejectBody = false;
    int rejectedChunks = 0;

    SECTION("accepts a chain rooted in the selected CA for the requested hostname")
    {
    }
    SECTION("rejects a server signed by an unknown CA")
    {
        auto otherKey = makeKey();
        auto otherRoot = makeCertificate(otherKey.get(), "TemplateTool unrelated CA", 3, nullptr, otherKey.get());
        pem = toPem(otherRoot.get());
        expected = CURLE_PEER_FAILED_VERIFICATION;
    }
    SECTION("rejects a trusted chain with a mismatched hostname")
    {
        host = "127.0.0.1";
        expected = CURLE_PEER_FAILED_VERIFICATION;
    }

    SECTION("aborts the transfer when the body sink fails")
    {
        rejectBody = true;
        expected = CURLE_WRITE_ERROR;
    }

    SECTION("parses Location without a space after the colon")
    {
        response = "HTTP/1.1 302 Found: redirect\r\n"
                   "Location:https://cdn.invalid:8443/asset\r\n"
                   "Content-Length: 0\r\nConnection: close\r\n\r\n";
        expectedStatus = 302;
        expectedHeaders = {{"Location", "https://cdn.invalid:8443/asset"},
                           {"Content-Length", "0"}, {"Connection", "close"}};
    }

    SECTION("trims spaces and tabs from Content-Range")
    {
        response = "HTTP/1.1 206 Partial Content\r\n"
                   "Content-Range:\t bytes 2-3/4 \t\r\n"
                   "X-Empty:\t \r\n"
                   "Content-Length: 2\r\nConnection: close\r\n\r\nok";
        expectedStatus = 206;
        expectedHeaders = {{"Content-Range", "bytes 2-3/4"}, {"X-Empty", ""},
                           {"Content-Length", "2"}, {"Connection", "close"}};
    }

    SECTION("separates 103 Early Hints from the final response")
    {
        response = "HTTP/1.1 103 Early Hints\r\n"
                   "Link: </early.css>; rel=preload; as=style\r\n\r\n"
                   "HTTP/1.1 200 OK\r\nContent-Length: 2\r\nConnection: close\r\n\r\nok";
    }

    const LoopbackTlsServer server(serverCertificate.get(), serverKey.get(), response);
    const auto port = std::to_string(server.port());
    const auto url = "https://" + host + ":" + port + "/";
    int receivedStatus = 0;
    NetworkHttpHeaders receivedHeaders;
    LibCurlEasyHandle handle([&](int status, const auto& headers, const auto&) {
        receivedStatus = status;
        receivedHeaders = headers;
    }, [&](const auto& buffer, bool) {
        if (rejectBody && !buffer.empty())
        {
            ++rejectedChunks;
            throw std::runtime_error("Download sink failed");
        }
    }, {});
    REQUIRE(handle.setCommonOptions() == CURLE_OK);
    handle.setURI(url);
    auto* curl = handle.getHandle();
    // Pin the fixture to loopback and ignore proxy environment settings.
    const auto hostMapping = "localhost:" + port + ":127.0.0.1";
    const std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)> resolved(
        curl_slist_append(nullptr, hostMapping.c_str()), &curl_slist_free_all);
    REQUIRE(resolved);
    REQUIRE(curl_easy_setopt(curl, CURLOPT_RESOLVE, resolved.get()) == CURLE_OK);
    REQUIRE(curl_easy_setopt(curl, CURLOPT_NOPROXY, "*") == CURLE_OK);
    REQUIRE(curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L) == CURLE_OK);
    curl_blob bundle{pem.data(), pem.size(), CURL_BLOB_COPY};
    // This fixture CA affects only this handle and is never installed on the host.
    REQUIRE(curl_easy_setopt(curl, CURLOPT_CAINFO_BLOB, &bundle) == CURLE_OK);
    CHECK(curl_easy_perform(curl) == expected);
    if (rejectBody)
    {
        CHECK(rejectedChunks == 1);
    }
    if (expected == CURLE_OK)
    {
        long status = 0;
        REQUIRE(curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status) == CURLE_OK);
        CHECK(status == expectedStatus);
        CHECK(receivedStatus == expectedStatus);
        CHECK(receivedHeaders == expectedHeaders);
    }
}

TEST_CASE("Network TLS requires HTTPS at every redirect hop", "[tls]")
{
    CurlRuntime runtime;
    NetworkHttpRequest request;
    request.setRequestUri("https://release.invalid/latest");
    REQUIRE(tryApplyHttpRedirect(request, 302, {{"Location", "/manifest.json"}}));
    REQUIRE(request.getRequestUri() == "https://release.invalid/manifest.json");
    REQUIRE(tryApplyHttpRedirect(request, 307, {{"Location", "https://cdn.invalid/asset"}}));
    REQUIRE(request.getRequestUri() == "https://cdn.invalid/asset");

    for (const int status : {301, 302, 303, 307, 308})
    {
        CAPTURE(status);
        CHECK_FALSE(tryApplyHttpRedirect(request, status, {{"Location", "http://cdn.invalid/cleartext"}}));
        CHECK(request.getRequestUri() == "https://cdn.invalid/asset");
    }

    CHECK_FALSE(tryApplyHttpRedirect(request, 302, {{"Location", "ftp://cdn.invalid/asset"}}));
    CHECK(request.getRequestUri() == "https://cdn.invalid/asset");

    request.setRequestUri("http://release.invalid/latest");
    CHECK_FALSE(tryApplyHttpRedirect(request, 301, {{"Location", "https://release.invalid/latest"}}));
}

TEST_CASE("Network TLS rejects a cleartext initial URL", "[tls]")
{
    CurlRuntime runtime;
    LibCurlEasyHandle handle({}, {}, {});
    REQUIRE(handle.setCommonOptions() == CURLE_OK);
    handle.setURI("http://127.0.0.1:1/");
    REQUIRE(curl_easy_setopt(handle.getHandle(), CURLOPT_NOPROXY, "*") == CURLE_OK);
    REQUIRE(curl_easy_setopt(handle.getHandle(), CURLOPT_TIMEOUT, 2L) == CURLE_OK);
    CHECK(curl_easy_perform(handle.getHandle()) == CURLE_UNSUPPORTED_PROTOCOL);
}
