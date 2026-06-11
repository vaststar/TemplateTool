#include <catch2/catch_test_macros.hpp>

#include <ucf/Services/UpgradeService/UpgradeModel.h>
#include <ucf/Services/UpgradeService/IUpgradeService.h>
#include <ucf/Services/UpgradeService/IUpgradeServiceCallback.h>

#include "fsm/UpgradeStates.h"
#include "fsm/UpgradeEvents.h"

#include <filesystem>
#include <string>
#include <vector>

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
ucf::service::upgrade::UpgradeContext makeTestContext()
{
    ucf::service::upgrade::UpgradeContext ctx;
    ctx.onStateChanged      = [](ucf::service::model::UpgradeState) {};
    ctx.onCheckCompleted    = [](const ucf::service::model::UpgradeCheckResult&) {};
    ctx.onDownloadProgress  = [](int64_t, int64_t) {};
    ctx.onError             = [](ucf::service::model::UpgradeErrorCode, const std::string&) {};
    ctx.triggerCheckForUpgrade = [](bool) {};
    ctx.triggerDownload     = [](const std::string&) {};
    ctx.triggerVerify       = [](const std::string&) {};
    ctx.triggerExtract      = [](const std::string&) {};
    ctx.triggerInstall      = [](const std::string&) {};
    ctx.triggerCancelDownload  = []() {};
    ctx.triggerHardReset = []() {};
    ctx.triggerSoftReset = []() {};
    return ctx;
}

ucf::service::model::UpgradeInfo makeSampleUpgradeInfo()
{
    ucf::service::model::PackageInfo pkg;
    pkg.downloadUrl = "https://example.com/upgrade.zip";
    pkg.sha256      = "abc123";
    pkg.sizeBytes   = 50'000'000;

    ucf::service::model::UpgradeInfo info;
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
    REQUIRE(static_cast<int>(ucf::service::model::UpgradeState::Idle) == 0);
    REQUIRE(static_cast<int>(ucf::service::model::UpgradeState::Checking) == 1);
    REQUIRE(static_cast<int>(ucf::service::model::UpgradeState::UpgradeAvailable) == 2);
    REQUIRE(static_cast<int>(ucf::service::model::UpgradeState::Downloading) == 3);
    REQUIRE(static_cast<int>(ucf::service::model::UpgradeState::Verifying) == 4);
    REQUIRE(static_cast<int>(ucf::service::model::UpgradeState::Extracting) == 5);
    REQUIRE(static_cast<int>(ucf::service::model::UpgradeState::ReadyToInstall) == 6);
    REQUIRE(static_cast<int>(ucf::service::model::UpgradeState::Installing) == 7);
    REQUIRE(static_cast<int>(ucf::service::model::UpgradeState::Failed) == 8);
}

// ============================================================================
// Section 2: FSM state transition tests
// ============================================================================
TEST_CASE("FSM starts in Idle state", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();
    ucf::service::upgrade::UpgradeFSM fsm(ctx);
    REQUIRE(fsm.template isIn<ucf::service::upgrade::Idle>());
}

TEST_CASE("FSM: Idle -> Checking on EvCheckRequested", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();
    ucf::service::upgrade::UpgradeFSM fsm(ctx);

    fsm.processEvent(ucf::service::upgrade::EvCheckRequested{.userTriggered = true});
    REQUIRE(fsm.template isIn<ucf::service::upgrade::Checking>());
}

TEST_CASE("FSM: Checking -> UpgradeAvailable on EvCheckSuccess", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();
    ucf::service::upgrade::UpgradeFSM fsm(ctx);

    fsm.processEvent(ucf::service::upgrade::EvCheckRequested{});
    REQUIRE(fsm.template isIn<ucf::service::upgrade::Checking>());

    fsm.processEvent(ucf::service::upgrade::EvCheckSuccess{.info = makeSampleUpgradeInfo()});
    REQUIRE(fsm.template isIn<ucf::service::upgrade::UpgradeAvailable>());
    REQUIRE(ctx.availableUpgrade.has_value());
    REQUIRE(ctx.availableUpgrade->version == "2.0.0");
}

TEST_CASE("FSM: Checking -> Idle on EvCheckNoUpgrade", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();

    bool checkCompletedFired = false;
    ctx.onCheckCompleted = [&](const ucf::service::model::UpgradeCheckResult& r) {
        checkCompletedFired = true;
        REQUIRE_FALSE(r.hasUpgrade);
    };

    ucf::service::upgrade::UpgradeFSM fsm(ctx);
    fsm.processEvent(ucf::service::upgrade::EvCheckRequested{});
    fsm.processEvent(ucf::service::upgrade::EvCheckNoUpgrade{});

    REQUIRE(fsm.template isIn<ucf::service::upgrade::Idle>());
    REQUIRE(checkCompletedFired);
}

TEST_CASE("FSM: Checking -> Failed on EvError", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();
    ucf::service::upgrade::UpgradeFSM fsm(ctx);

    fsm.processEvent(ucf::service::upgrade::EvCheckRequested{});
    fsm.processEvent(ucf::service::upgrade::EvError{.code = ucf::service::model::UpgradeErrorCode::NetworkError, .message = "timeout"});

    REQUIRE(fsm.template isIn<ucf::service::upgrade::Failed>());
}

TEST_CASE("FSM: UpgradeAvailable -> Downloading on EvDownloadStart", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();
    ctx.availableUpgrade = makeSampleUpgradeInfo();
    ucf::service::upgrade::UpgradeFSM fsm(ctx);

    fsm.processEvent(ucf::service::upgrade::EvCheckRequested{});
    fsm.processEvent(ucf::service::upgrade::EvCheckSuccess{.info = makeSampleUpgradeInfo()});
    fsm.processEvent(ucf::service::upgrade::EvDownloadStart{});

    REQUIRE(fsm.template isIn<ucf::service::upgrade::Downloading>());
}

TEST_CASE("FSM: UpgradeAvailable -> Idle on EvDismiss", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();
    ucf::service::upgrade::UpgradeFSM fsm(ctx);

    fsm.processEvent(ucf::service::upgrade::EvCheckRequested{});
    fsm.processEvent(ucf::service::upgrade::EvCheckSuccess{.info = makeSampleUpgradeInfo()});
    fsm.processEvent(ucf::service::upgrade::EvDismiss{});

    REQUIRE(fsm.template isIn<ucf::service::upgrade::Idle>());
}

TEST_CASE("FSM: Downloading -> Verifying on EvDownloadDone", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();
    ctx.availableUpgrade = makeSampleUpgradeInfo();
    ucf::service::upgrade::UpgradeFSM fsm(ctx);

    fsm.processEvent(ucf::service::upgrade::EvCheckRequested{});
    fsm.processEvent(ucf::service::upgrade::EvCheckSuccess{.info = makeSampleUpgradeInfo()});
    fsm.processEvent(ucf::service::upgrade::EvDownloadStart{});
    fsm.processEvent(ucf::service::upgrade::EvDownloadDone{.filePath = "/tmp/test.zip"});

    REQUIRE(fsm.template isIn<ucf::service::upgrade::Verifying>());
    REQUIRE(ctx.downloadedFilePath == "/tmp/test.zip");
}

TEST_CASE("FSM: Downloading -> Idle on EvCancel (soft reset)", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();
    ctx.availableUpgrade = makeSampleUpgradeInfo();

    bool softResetCalled = false;
    ctx.triggerSoftReset = [&]() { softResetCalled = true; };

    ucf::service::upgrade::UpgradeFSM fsm(ctx);

    fsm.processEvent(ucf::service::upgrade::EvCheckRequested{});
    fsm.processEvent(ucf::service::upgrade::EvCheckSuccess{.info = makeSampleUpgradeInfo()});
    fsm.processEvent(ucf::service::upgrade::EvDownloadStart{});
    fsm.processEvent(ucf::service::upgrade::EvCancel{});

    REQUIRE(fsm.template isIn<ucf::service::upgrade::Idle>());
    REQUIRE(softResetCalled);
}

TEST_CASE("FSM: Verifying -> ReadyToInstall on EvVerifyOk", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();
    ctx.availableUpgrade = makeSampleUpgradeInfo();
    ucf::service::upgrade::UpgradeFSM fsm(ctx);

    fsm.processEvent(ucf::service::upgrade::EvCheckRequested{});
    fsm.processEvent(ucf::service::upgrade::EvCheckSuccess{.info = makeSampleUpgradeInfo()});
    fsm.processEvent(ucf::service::upgrade::EvDownloadStart{});
    fsm.processEvent(ucf::service::upgrade::EvDownloadDone{.filePath = "/tmp/test.zip"});
    fsm.processEvent(ucf::service::upgrade::EvVerifyOk{});

    REQUIRE(fsm.template isIn<ucf::service::upgrade::ReadyToInstall>());
}

TEST_CASE("FSM: ReadyToInstall -> Installing on EvInstallStart", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();
    ctx.availableUpgrade = makeSampleUpgradeInfo();
    ucf::service::upgrade::UpgradeFSM fsm(ctx);

    fsm.processEvent(ucf::service::upgrade::EvCheckRequested{});
    fsm.processEvent(ucf::service::upgrade::EvCheckSuccess{.info = makeSampleUpgradeInfo()});
    fsm.processEvent(ucf::service::upgrade::EvDownloadStart{});
    fsm.processEvent(ucf::service::upgrade::EvDownloadDone{.filePath = "/tmp/test.zip"});
    fsm.processEvent(ucf::service::upgrade::EvVerifyOk{});
    fsm.processEvent(ucf::service::upgrade::EvInstallStart{});

    REQUIRE(fsm.template isIn<ucf::service::upgrade::Installing>());
}

TEST_CASE("FSM: Failed -> Idle on EvDismiss", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();
    ucf::service::upgrade::UpgradeFSM fsm(ctx);

    fsm.processEvent(ucf::service::upgrade::EvCheckRequested{});
    fsm.processEvent(ucf::service::upgrade::EvError{.code = ucf::service::model::UpgradeErrorCode::NetworkError, .message = "err"});
    REQUIRE(fsm.template isIn<ucf::service::upgrade::Failed>());

    fsm.processEvent(ucf::service::upgrade::EvDismiss{});
    REQUIRE(fsm.template isIn<ucf::service::upgrade::Idle>());
}

TEST_CASE("FSM: state change notifications fire correctly", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();

    std::vector<ucf::service::model::UpgradeState> stateLog;
    ctx.onStateChanged = [&](ucf::service::model::UpgradeState s) { stateLog.push_back(s); };

    ucf::service::upgrade::UpgradeFSM fsm(ctx);
    // Initial onEnter fires for Idle
    REQUIRE(stateLog.size() == 1);
    REQUIRE(stateLog.back() == ucf::service::model::UpgradeState::Idle);

    fsm.processEvent(ucf::service::upgrade::EvCheckRequested{});
    REQUIRE(stateLog.back() == ucf::service::model::UpgradeState::Checking);

    fsm.processEvent(ucf::service::upgrade::EvCheckNoUpgrade{});
    // Goes back to Idle
    REQUIRE(stateLog.back() == ucf::service::model::UpgradeState::Idle);
}

TEST_CASE("FSM: download progress stays in Downloading", "[UpgradeService][FSM]")
{
    auto ctx = makeTestContext();
    ctx.availableUpgrade = makeSampleUpgradeInfo();

    std::vector<std::pair<int64_t, int64_t>> progressLog;
    ctx.onDownloadProgress = [&](int64_t cur, int64_t tot) {
        progressLog.emplace_back(cur, tot);
    };

    ucf::service::upgrade::UpgradeFSM fsm(ctx);

    fsm.processEvent(ucf::service::upgrade::EvCheckRequested{});
    fsm.processEvent(ucf::service::upgrade::EvCheckSuccess{.info = makeSampleUpgradeInfo()});
    fsm.processEvent(ucf::service::upgrade::EvDownloadStart{});

    fsm.processEvent(ucf::service::upgrade::EvProgress{.current = 100, .total = 1000});
    fsm.processEvent(ucf::service::upgrade::EvProgress{.current = 500, .total = 1000});
    fsm.processEvent(ucf::service::upgrade::EvProgress{.current = 1000, .total = 1000});

    REQUIRE(fsm.template isIn<ucf::service::upgrade::Downloading>());
    REQUIRE(progressLog.size() == 3);
    REQUIRE(progressLog[0].first == 100);
    REQUIRE(progressLog[2].first == 1000);
}


