#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/CoreFramework/FakeCoreFramework.h>
#include <ucf/services/ClientInfoService/ClientInfoServiceCreator.h>
#include <ucf/services/ClientInfoService/IClientInfoService.h>

#if defined(_WIN32)
#include <filesystem>
#include <string>

#include <catch2/generators/catch_generators.hpp>
#include <windows.h>
#endif

TEST_CASE("ClientInfoService can be created through its public creator", "[ClientInfoService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    REQUIRE(fakeCoreFramework != nullptr);

    auto clientInfoService = ucf::service::impl::createClientInfoService(fakeCoreFramework);
    REQUIRE(clientInfoService != nullptr);
    REQUIRE(clientInfoService->getServiceName() == "ClientInfoService");
}

#if defined(_WIN32)

namespace {

class ScopedEnvironmentVariable
{
public:
    ScopedEnvironmentVariable(const wchar_t* name, const wchar_t* value)
        : mName(name)
    {
        const DWORD size = GetEnvironmentVariableW(mName, nullptr, 0);
        if (size != 0)
        {
            mPreviousValue.resize(size);
            REQUIRE(GetEnvironmentVariableW(mName, mPreviousValue.data(), size) < size);
        }
        REQUIRE(SetEnvironmentVariableW(mName, value) != 0);
    }

    ~ScopedEnvironmentVariable()
    {
        SetEnvironmentVariableW(mName, mPreviousValue.empty() ? nullptr : mPreviousValue.c_str());
    }

private:
    const wchar_t* mName;
    std::wstring mPreviousValue;
};

} // namespace

TEST_CASE("ClientInfoService returns UTF-8 cache paths for a Chinese Windows user directory", "[ClientInfoService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    auto clientInfoService = ucf::service::impl::createClientInfoService(fakeCoreFramework);
    REQUIRE(clientInfoService);

    const auto userProfile = std::filesystem::path(L"C:\\Users\\\u4E2D\u6587\u7528\u6237");
    const auto localAppData = userProfile / L"AppData" / L"Local";
    const bool useLocalAppData = GENERATE(true, false);
    CAPTURE(useLocalAppData);
    const ScopedEnvironmentVariable profile(L"USERPROFILE", userProfile.c_str());
    const ScopedEnvironmentVariable local(L"LOCALAPPDATA", useLocalAppData ? localAppData.c_str() : L"");

    const auto cacheUtf8 = clientInfoService->getAppCacheStoragePath();
    const auto miniAppCacheUtf8 = clientInfoService->getMiniAppCacheStoragePath();
    const std::filesystem::path cache(std::u8string(cacheUtf8.begin(), cacheUtf8.end()));
    const std::filesystem::path miniAppCache(std::u8string(miniAppCacheUtf8.begin(), miniAppCacheUtf8.end()));
    CHECK(cache.parent_path().parent_path() == (useLocalAppData ? localAppData : userProfile));
    CHECK(cache.filename() == L"app_cache");
    CHECK(miniAppCache == cache / L"mini_app");
}

#endif
