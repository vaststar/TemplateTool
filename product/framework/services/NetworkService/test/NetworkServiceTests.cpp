#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <fakes/ucf/CoreFramework/FakeCoreFramework.h>
#include <ucf/services/NetworkService/NetworkServiceCreator.h>
#include <ucf/services/NetworkService/INetworkService.h>
#include <ucf/services/NetworkService/model/HttpDownloadToFileRequest.h>
#include <ucf/services/NetworkService/model/HttpDownloadToFileResponse.h>
#include <ucf/services/NetworkService/model/HttpDownloadToMemoryRequest.h>
#include <ucf/services/NetworkService/model/HttpRestRequest.h>
#include <ucf/infrastructure/NetworkClient/NetworkModelTypes/http/NetworkHttpRequest.h>
#include <ucf/utilities/FilePathUtils/FilePathUtils.h>

#include "NetworkHttpHandler/NetworkHttpDownloadToFileHandler.h"
#include "NetworkHttpHandler/NetworkHttpDownloadToMemoryHandler.h"
#include "NetworkHttpHandler/NetworkHttpRestHandler.h"

namespace {

struct TemporaryDirectory
{
    TemporaryDirectory()
        : path(std::filesystem::temp_directory_path() /
               ucf::service::network::http::HttpDownloadToFileRequest{}.getRequestId())
    {
        REQUIRE(std::filesystem::create_directory(path));
    }

    ~TemporaryDirectory()
    {
        std::error_code error;
        std::filesystem::remove_all(path, error);
    }

    std::filesystem::path path;
};

void seedPartialFile(const std::filesystem::path& path)
{
    std::ofstream file(path, std::ios::binary);
    file << "ab";
    file.close();
    REQUIRE(file.good());
}

} // namespace

TEST_CASE("NetworkService can be created through its public creator", "[NetworkService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    REQUIRE(fakeCoreFramework != nullptr);

    auto service = ucf::service::impl::createNetworkService(fakeCoreFramework);
    REQUIRE(service != nullptr);
    REQUIRE(service->getServiceName() == "NetworkService");
}

TEST_CASE("Network TLS redirect handlers reject cleartext targets", "[NetworkService][tls]")
{
    using namespace ucf::service::network::http;
    const auto service = ucf::service::impl::createNetworkService({});
    REQUIRE(service);

    constexpr auto secureUrl = "https://release.invalid/asset";
    const NetworkHttpHeaders insecureRedirect{{"Location", "http://release.invalid/asset"}};

    SECTION("REST requests")
    {
        HttpRestRequest request(HTTPMethod::GET, secureUrl, {}, "", 5);
        NetworkHttpRestHandler handler(request, {});
        handler.setResponseHeader(302, insecureRedirect, {});
        CHECK_FALSE(handler.tryPrepareRedirectRequest());
        CHECK(handler.getHttpRequest().getRequestUri() == secureUrl);
    }
    SECTION("File downloads")
    {
        HttpDownloadToFileRequest request(secureUrl, {}, 5, "");
        NetworkHttpDownloadToFileHandler handler(request, {});
        handler.setResponseHeader(307, insecureRedirect, {});
        CHECK_FALSE(handler.tryPrepareRedirectRequest());
        CHECK(handler.getHttpRequest().getRequestUri() == secureUrl);
    }
    SECTION("Memory downloads")
    {
        HttpDownloadToMemoryRequest request(secureUrl, {}, 5);
        NetworkHttpDownloadToMemoryHandler handler(request, {});
        handler.setResponseHeader(302, insecureRedirect, {});
        CHECK_FALSE(handler.tryPrepareRedirectRequest());
        CHECK(handler.getHttpRequest().getRequestUri() == secureUrl);
    }
}

TEST_CASE("NetworkService file download stops when its destination cannot be opened", "[NetworkService]")
{
    using namespace ucf::service::network::http;
    using CurlError = ucf::infrastructure::network::http::ResponseErrorStruct;
    using CurlErrorType = ucf::infrastructure::network::http::ResponseErrorType;

    HttpDownloadToFileRequest request("https://release.invalid/asset", {}, 5, "");
    int callbackCount = 0;
    std::optional<ResponseErrorStruct> error;
    NetworkHttpDownloadToFileHandler handler(request, [&](const HttpDownloadToFileResponse& response) {
        ++callbackCount;
        CHECK(response.isFinished());
        CHECK(response.getCurrentSize() == 0);
        error = response.getErrorData();
    });

    handler.setResponseHeader(200, {}, {});
    REQUIRE_THROWS_AS(handler.appendResponseBody({1, 2, 3}, false), std::runtime_error);
    CHECK(callbackCount == 0);

    handler.setResponseHeader(200, {}, CurlError{0, CurlErrorType::OtherError, "Response body callback failed"});
    handler.completeResponse({});

    CHECK(callbackCount == 1);
    REQUIRE(error.has_value());
    CHECK(error->errorType == ResponseErrorType::OtherError);
    CHECK(error->errorDescription == "Cannot open download file");
}

TEST_CASE("NetworkService opens UTF-8 download paths and caps file size", "[NetworkService]")
{
    using namespace ucf::service::network::http;
    const auto status = GENERATE(200, 206);
    const bool exceedsLimit = GENERATE(false, true);
    const bool afterFirstChunk = GENERATE(false, true);
    const bool chinesePath = GENERATE(false, true);
    CAPTURE(status, exceedsLimit, afterFirstChunk, chinesePath);

    const TemporaryDirectory temporary;
    const auto filePath = temporary.path / std::filesystem::path(chinesePath ? u8"中文.part" : u8"download.part");
    seedPartialFile(filePath);

    constexpr std::size_t sizeLimit = 5;
    // Both requests carry Range, but only a 206 response can append to the partial file.
    HttpDownloadToFileRequest request("https://release.invalid/asset",
        {{"Range", "bytes=2-"}}, 5, ucf::utilities::FilePathUtils::utf8FromPath(filePath));
    request.setMaxFileSize(sizeLimit);
    int finishedCount = 0;
    std::optional<ResponseErrorStruct> error;
    NetworkHttpDownloadToFileHandler handler(request, [&](const HttpDownloadToFileResponse& response) {
        if (response.isFinished())
        {
            ++finishedCount;
            error = response.getErrorData();
        }
    });

    const bool resume = status == 206;
    const auto responseHeaders = resume
        ? NetworkHttpHeaders{{"Content-Range", "bytes 2-4/5"}}
        : NetworkHttpHeaders{};
    handler.setResponseHeader(status, responseHeaders, {});
    if (afterFirstChunk)
    {
        handler.appendResponseBody({'x'}, false);
    }
    const auto sizeBeforeFinalChunk = (resume ? 2u : 0u) + (afterFirstChunk ? 1u : 0u);
    const ByteBuffer finalChunk(sizeLimit - sizeBeforeFinalChunk + (exceedsLimit ? 1u : 0u), 'y');
    if (exceedsLimit)
    {
        REQUIRE_THROWS_AS(handler.appendResponseBody(finalChunk, false), std::runtime_error);
    }
    else
    {
        REQUIRE_NOTHROW(handler.appendResponseBody(finalChunk, false));
    }
    handler.completeResponse({});

    CHECK(finishedCount == 1);
    const auto sizeBeforeRejectedWrite = afterFirstChunk ? sizeBeforeFinalChunk : 2u;
    CHECK(std::filesystem::file_size(filePath) == (exceedsLimit ? sizeBeforeRejectedWrite : sizeLimit));
    if (exceedsLimit)
    {
        REQUIRE(error.has_value());
        CHECK(error->errorType == ResponseErrorType::OtherError);
        CHECK(error->errorDescription == "Download exceeds expected file size");
    }
    else
    {
        CHECK_FALSE(error.has_value());
    }
}

TEST_CASE("NetworkService validates resumed download segment length", "[NetworkService]")
{
    using namespace ucf::service::network::http;
    using CurlError = ucf::infrastructure::network::http::ResponseErrorStruct;
    using CurlErrorType = ucf::infrastructure::network::http::ResponseErrorType;

    const TemporaryDirectory temporary;
    const auto filePath = temporary.path / "download.part";
    seedPartialFile(filePath);
    HttpDownloadToFileRequest request("https://release.invalid/asset",
        {{"Range", "bytes=2-"}}, 5, ucf::utilities::FilePathUtils::utf8FromPath(filePath));
    request.setMaxFileSize(6);

    int finishedCount = 0;
    std::optional<ResponseErrorStruct> error;
    NetworkHttpDownloadToFileHandler handler(request, [&](const HttpDownloadToFileResponse& response) {
        if (response.isFinished())
        {
            ++finishedCount;
            error = response.getErrorData();
        }
    });
    const auto contentRange = GENERATE("bytes 2-3/6", "Bytes 2-3/*");
    const auto bodySize = GENERATE(0u, 1u, 2u, 3u);
    CAPTURE(contentRange, bodySize);
    const NetworkHttpHeaders headers{{"Content-Range", contentRange}};
    handler.setResponseHeader(206, headers, {});

    if (bodySize > 0)
    {
        handler.appendResponseBody({'c'}, false);
    }
    const ByteBuffer lastChunk(bodySize > 0 ? bodySize - 1 : 0, 'd');
    const bool oversized = bodySize > 2;
    if (oversized)
    {
        REQUIRE_THROWS_AS(handler.appendResponseBody(lastChunk, false), std::runtime_error);
        CHECK(finishedCount == 0);
        handler.setResponseHeader(206, headers, CurlError{0, CurlErrorType::OtherError,
            "Response body callback failed"});
    }
    else
    {
        handler.appendResponseBody(lastChunk, false);
        handler.appendResponseBody({}, true);
    }
    handler.completeResponse({});

    CHECK(finishedCount == 1);
    CHECK(std::filesystem::file_size(filePath) == 2u + (oversized ? 1u : bodySize));
    if (bodySize == 2)
    {
        CHECK_FALSE(error.has_value());
    }
    else
    {
        REQUIRE(error.has_value());
        CHECK(error->errorType == ResponseErrorType::OtherError);
        CHECK(error->errorDescription == (oversized
            ? "Download exceeds Content-Range length"
            : "Download ended before Content-Range was complete"));
    }
}
