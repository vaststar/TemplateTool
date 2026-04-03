#include <catch2/catch_test_macros.hpp>

#include <ucf/Services/UpgradeService/UpgradeModel.h>
#include <ucf/Services/UpgradeService/IUpgradeService.h>
#include <ucf/Services/UpgradeService/IUpgradeServiceCallback.h>

#include "fsm/UpgradeStates.h"
#include "fsm/UpgradeEvents.h"

#include <filesystem>
#include <string>
#include <vector>

using namespace ucf::service;
using namespace ucf::service::upgrade;
using namespace ucf::service::model;

// ============================================================================
// Helper: Temporary directory RAII wrapper
// ============================================================================
namespace {

class TempDirectory
{
public:
    explicit TempDirectory(const std::string& prefix = "upgrade_test_")
    {
        mPath = std::filesystem::temp_directory_path() / (prefix + std::to_string(std::rand()));
        std::filesystem::create_directories(mPath);
    }

    ~TempDirectory()
    {
        std::error_code ec;
        std::filesystem::remove_all(mPath, ec);
    }

    TempDirectory(const TempDirectory&) = delete;
    TempDirectory& operator=(const TempDirectory&) = delete;

    [[nodiscard]] const std::filesystem::path& path() const { return mPath; }

private:
    std::filesystem::path mPath;
};

// Helper: Create a minimal UpgradeContext with no-op callbacks
UpgradeContext makeTestContext()
{
    UpgradeContext ctx;
    ctx.onStateChanged      = [](UpgradeState) {};
    ctx.onCheckCompleted    = [](const UpgradeCheckResult&) {};
    ctx.onDownloadProgress  = [](int64_t, int64_t) {};
    ctx.onError             = [](UpgradeErrorCode, const std::string&) {};
    ctx.triggerCheckForUpgrade = [](bool) {};
    ctx.triggerDownload     = [](const std::string&) {};
    ctx.triggerVerify       = [](const std::string&) {};
    ctx.triggerInstall      = [](const std::string&) {};
    ctx.triggerCancelDownload  = []() {};
    ctx.triggerResetManagers   = []() {};
    ctx.triggerSoftResetManagers = []() {};
    return ctx;
}

UpgradeInfo makeSampleUpgradeInfo()
{
    PackageInfo pkg;
    pkg.downloadUrl = "https://example.com/upgrade.zip";
    pkg.sha256      = "abc123";
    pkg.sizeBytes   = 50'000'000;

    UpgradeInfo info;
    info.version     = "2.0.0";
    info.releaseNotes = "Bug fixes";
    info.mandatory   = false;
    info.package     = pkg;
    return info;
}

} // anonymous namespace

// ============================================================================
// Section 1: UpgradeModel tests
// ============================================================================
TEST_CASE("UpgradeModel enums have expected values", "[UpgradeService][Model]")
{
    REQUIRE(static_cast<int>(UpgradeState::Idle) == 0);
    REQUIRE(static_cast<int>(UpgradeState::Checking) == 1);
    REQUIRE(static_cast<int>(UpgradeState::UpgradeAvailable) == 2);
    REQUIRE(static_cast<int>(UpgradeState::Downloading) == 3);
    REQUIRE(static_cast<int>(UpgradeState::Verifying) == 4);
    REQUIRE(static_cast<int>(UpgradeState::ReadyToInstall) == 5);
    REQUIRE(static_cast<int>(UpgradeState::Installing) == 6);
    REQUIRE(static_cast<int>(UpgradeState::Failed) == 7);
}

// ============================================================================
// Section 2: FSM state transition tests
// ============================================================================
TEST_CASE("FSM starts in Idle state", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();
    UpgradeFSM fsm(ctx);
    REQUIRE(fsm.template isIn<Idle>());
}

TEST_CASE("FSM: Idle -> Checking on EvCheckRequested", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();
    UpgradeFSM fsm(ctx);

    fsm.processEvent(EvCheckRequested{.userTriggered = true});
    REQUIRE(fsm.template isIn<Checking>());
}

TEST_CASE("FSM: Checking -> UpgradeAvailable on EvCheckSuccess", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();
    UpgradeFSM fsm(ctx);

    fsm.processEvent(EvCheckRequested{});
    REQUIRE(fsm.template isIn<Checking>());

    fsm.processEvent(EvCheckSuccess{.info = makeSampleUpgradeInfo()});
    REQUIRE(fsm.template isIn<UpgradeAvailable>());
    REQUIRE(ctx.availableUpgrade.has_value());
    REQUIRE(ctx.availableUpgrade->version == "2.0.0");
}

TEST_CASE("FSM: Checking -> Idle on EvCheckNoUpgrade", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();

    bool checkCompletedFired = false;
    ctx.onCheckCompleted = [&](const UpgradeCheckResult& r) {
        checkCompletedFired = true;
        REQUIRE_FALSE(r.hasUpgrade);
    };

    UpgradeFSM fsm(ctx);
    fsm.processEvent(EvCheckRequested{});
    fsm.processEvent(EvCheckNoUpgrade{});

    REQUIRE(fsm.template isIn<Idle>());
    REQUIRE(checkCompletedFired);
}

TEST_CASE("FSM: Checking -> Failed on EvError", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();
    UpgradeFSM fsm(ctx);

    fsm.processEvent(EvCheckRequested{});
    fsm.processEvent(EvError{.code = UpgradeErrorCode::NetworkError, .message = "timeout"});

    REQUIRE(fsm.template isIn<Failed>());
}

TEST_CASE("FSM: UpgradeAvailable -> Downloading on EvDownloadStart", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();
    ctx.availableUpgrade = makeSampleUpgradeInfo();
    UpgradeFSM fsm(ctx);

    fsm.processEvent(EvCheckRequested{});
    fsm.processEvent(EvCheckSuccess{.info = makeSampleUpgradeInfo()});
    fsm.processEvent(EvDownloadStart{});

    REQUIRE(fsm.template isIn<Downloading>());
}

TEST_CASE("FSM: UpgradeAvailable -> Idle on EvRemindLater", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();
    UpgradeFSM fsm(ctx);

    fsm.processEvent(EvCheckRequested{});
    fsm.processEvent(EvCheckSuccess{.info = makeSampleUpgradeInfo()});
    fsm.processEvent(EvRemindLater{});

    REQUIRE(fsm.template isIn<Idle>());
}

TEST_CASE("FSM: Downloading -> Verifying on EvDownloadDone", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();
    ctx.availableUpgrade = makeSampleUpgradeInfo();
    UpgradeFSM fsm(ctx);

    fsm.processEvent(EvCheckRequested{});
    fsm.processEvent(EvCheckSuccess{.info = makeSampleUpgradeInfo()});
    fsm.processEvent(EvDownloadStart{});
    fsm.processEvent(EvDownloadDone{.filePath = "/tmp/test.zip"});

    REQUIRE(fsm.template isIn<Verifying>());
    REQUIRE(ctx.downloadedFilePath == "/tmp/test.zip");
}

TEST_CASE("FSM: Downloading -> Idle on EvCancel (sets keepPartialDownload)", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();
    ctx.availableUpgrade = makeSampleUpgradeInfo();

    bool softResetCalled = false;
    ctx.triggerSoftResetManagers = [&]() { softResetCalled = true; };

    UpgradeFSM fsm(ctx);

    fsm.processEvent(EvCheckRequested{});
    fsm.processEvent(EvCheckSuccess{.info = makeSampleUpgradeInfo()});
    fsm.processEvent(EvDownloadStart{});
    fsm.processEvent(EvCancel{});

    REQUIRE(fsm.template isIn<Idle>());
    REQUIRE(softResetCalled);
}

TEST_CASE("FSM: Verifying -> ReadyToInstall on EvVerifyOk", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();
    ctx.availableUpgrade = makeSampleUpgradeInfo();
    UpgradeFSM fsm(ctx);

    fsm.processEvent(EvCheckRequested{});
    fsm.processEvent(EvCheckSuccess{.info = makeSampleUpgradeInfo()});
    fsm.processEvent(EvDownloadStart{});
    fsm.processEvent(EvDownloadDone{.filePath = "/tmp/test.zip"});
    fsm.processEvent(EvVerifyOk{});

    REQUIRE(fsm.template isIn<ReadyToInstall>());
}

TEST_CASE("FSM: ReadyToInstall -> Installing on EvInstallStart", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();
    ctx.availableUpgrade = makeSampleUpgradeInfo();
    UpgradeFSM fsm(ctx);

    fsm.processEvent(EvCheckRequested{});
    fsm.processEvent(EvCheckSuccess{.info = makeSampleUpgradeInfo()});
    fsm.processEvent(EvDownloadStart{});
    fsm.processEvent(EvDownloadDone{.filePath = "/tmp/test.zip"});
    fsm.processEvent(EvVerifyOk{});
    fsm.processEvent(EvInstallStart{});

    REQUIRE(fsm.template isIn<Installing>());
}

TEST_CASE("FSM: Failed -> Idle on EvReset", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();
    UpgradeFSM fsm(ctx);

    fsm.processEvent(EvCheckRequested{});
    fsm.processEvent(EvError{.code = UpgradeErrorCode::NetworkError, .message = "err"});
    REQUIRE(fsm.template isIn<Failed>());

    fsm.processEvent(EvReset{});
    REQUIRE(fsm.template isIn<Idle>());
}

TEST_CASE("FSM: state change notifications fire correctly", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();

    std::vector<UpgradeState> stateLog;
    ctx.onStateChanged = [&](UpgradeState s) { stateLog.push_back(s); };

    UpgradeFSM fsm(ctx);
    // Initial onEnter fires for Idle
    REQUIRE(stateLog.size() == 1);
    REQUIRE(stateLog.back() == UpgradeState::Idle);

    fsm.processEvent(EvCheckRequested{});
    REQUIRE(stateLog.back() == UpgradeState::Checking);

    fsm.processEvent(EvCheckNoUpgrade{});
    // Goes back to Idle
    REQUIRE(stateLog.back() == UpgradeState::Idle);
}

TEST_CASE("FSM: download progress stays in Downloading", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();
    ctx.availableUpgrade = makeSampleUpgradeInfo();

    std::vector<std::pair<int64_t, int64_t>> progressLog;
    ctx.onDownloadProgress = [&](int64_t cur, int64_t tot) {
        progressLog.emplace_back(cur, tot);
    };

    UpgradeFSM fsm(ctx);

    fsm.processEvent(EvCheckRequested{});
    fsm.processEvent(EvCheckSuccess{.info = makeSampleUpgradeInfo()});
    fsm.processEvent(EvDownloadStart{});

    fsm.processEvent(EvProgress{.current = 100, .total = 1000});
    fsm.processEvent(EvProgress{.current = 500, .total = 1000});
    fsm.processEvent(EvProgress{.current = 1000, .total = 1000});

    REQUIRE(fsm.template isIn<Downloading>());
    REQUIRE(progressLog.size() == 3);
    REQUIRE(progressLog[0].first == 100);
    REQUIRE(progressLog[2].first == 1000);
}


