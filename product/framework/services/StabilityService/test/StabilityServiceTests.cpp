#include <catch2/catch_test_macros.hpp>
#include <trompeloeil.hpp>
#include <trompeloeil/matcher/any.hpp>

#include <fakes/ucf/CoreFramework/FakeCoreFramework.h>
#include <fakes/ucf/Services/FakeClientInfoService.h>
#include "StabilityService.h"

#include <filesystem>
#include <fstream>
#include <typeindex>
#include <vector>
#include <memory>

using namespace ucf::service;
using namespace ucf::framework::fakes;
using namespace ucf::service::fakes;
using trompeloeil::_;

namespace {

// Testable subclass that exposes initService
class TestableStabilityService : public StabilityService
{
public:
    using StabilityService::StabilityService;
    using StabilityService::initService;  // Expose protected method
};

// Helper to create a temporary test directory
class TempDirectory
{
public:
    TempDirectory(const std::string& prefix = "stability_test_")
    {
        mPath = std::filesystem::temp_directory_path() / (prefix + std::to_string(std::rand()));
        std::filesystem::create_directories(mPath);
    }
    
    ~TempDirectory()
    {
        std::error_code ec;
        std::filesystem::remove_all(mPath, ec);
    }
    
    const std::filesystem::path& path() const { return mPath; }
    std::string pathString() const { return mPath.string(); }
    
private:
    std::filesystem::path mPath;
};

// Helper to write a test log file
void writeTestLog(const std::filesystem::path& dir, const std::string& filename, const std::string& content)
{
    std::filesystem::create_directories(dir);
    std::ofstream file(dir / filename);
    file << content;
}

// Sample crash log content
const char* kSampleCrashLog = R"(=== Crash Report ===
Timestamp: 2026-02-03T10:30:00
Product: TestProduct
Version: 1.0.0
Signal: 11
Signal Name: SIGSEGV

Stack Trace:
#0: test_function()
#1: main()
)";

// Sample hang log content
const char* kSampleHangLog = R"(=== Hang Report ===
Timestamp: 2026-02-03T10:30:00
Product: TestProduct
Version: 1.0.0
Hang Duration: 5000 ms
Recovered: true

Stack Trace:
#0: blocking_function()
#1: main()
)";

} // anonymous namespace

//=============================================================================
// Test Fixture
//=============================================================================

class StabilityServiceTestFixture
{
public:
    StabilityServiceTestFixture()
        : mCrashDir{"crash_test_"}
        , mHangDir{"hang_test_"}
        , mCrashDirPath(mCrashDir.pathString())
        , mHangDirPath(mHangDir.pathString())
        , mFakeCoreFramework(std::make_shared<FakeCoreFramework>())
        , mFakeClientInfoService(std::make_shared<FakeClientInfoService>())
    {
        // Use NAMED_ALLOW_CALL to store expectations that persist
        mExpectations.push_back(
            NAMED_ALLOW_CALL(*mFakeCoreFramework, getServiceInternal(ANY(std::type_index)))
                .WITH(_1 == std::type_index(typeid(IClientInfoService)))
                .LR_RETURN(mFakeClientInfoService));
        
        mExpectations.push_back(
            NAMED_ALLOW_CALL(*mFakeClientInfoService, getAppCrashStoragePath())
                .LR_RETURN(mCrashDirPath));
        
        mExpectations.push_back(
            NAMED_ALLOW_CALL(*mFakeClientInfoService, getAppHangStoragePath())
                .LR_RETURN(mHangDirPath));
        
        mExpectations.push_back(
            NAMED_ALLOW_CALL(*mFakeClientInfoService, getApplicationVersion())
                .RETURN(model::Version{"1", "0", "0", "0"}));
        
        mExpectations.push_back(
            NAMED_ALLOW_CALL(*mFakeClientInfoService, getProductInfo())
                .RETURN(model::ProductInfo{"TestCompany", "Copyright", "TestProduct", "Description"}));
    }
    
    std::shared_ptr<TestableStabilityService> createService()
    {
        auto service = std::make_shared<TestableStabilityService>(mFakeCoreFramework);
        service->initService();  // Initialize managers
        return service;
    }
    
    const std::filesystem::path& crashDir() const { return mCrashDir.path(); }
    const std::filesystem::path& hangDir() const { return mHangDir.path(); }
    
protected:
    TempDirectory mCrashDir;
    TempDirectory mHangDir;
    std::string mCrashDirPath;
    std::string mHangDirPath;
    std::shared_ptr<FakeCoreFramework> mFakeCoreFramework;
    std::shared_ptr<FakeClientInfoService> mFakeClientInfoService;
    std::vector<std::unique_ptr<trompeloeil::expectation>> mExpectations;
};

//=============================================================================
// Basic Tests
//=============================================================================

TEST_CASE("StabilityService can be created", "[StabilityService]")
{
    StabilityServiceTestFixture fixture;
    auto service = fixture.createService();
    
    REQUIRE(service != nullptr);
    REQUIRE(service->getServiceName() == "StabilityService");
}

//=============================================================================
// Crash Report Tests
//=============================================================================

TEST_CASE("StabilityService - no pending crash report when directory is empty", "[StabilityService][Crash]")
{
    StabilityServiceTestFixture fixture;
    auto service = fixture.createService();
    
    REQUIRE_FALSE(service->hasPendingCrashReport());
    REQUIRE(service->getCrashReportFiles().empty());
    REQUIRE_FALSE(service->getLastCrashInfo().has_value());
}

TEST_CASE("StabilityService - detects pending crash report", "[StabilityService][Crash]")
{
    StabilityServiceTestFixture fixture;
    
    // Write a crash log before creating service
    writeTestLog(fixture.crashDir(), "crash_2026-02-03_103000.crash", kSampleCrashLog);
    
    auto service = fixture.createService();
    
    REQUIRE(service->hasPendingCrashReport());
    REQUIRE(service->getCrashReportFiles().size() == 1);
}

TEST_CASE("StabilityService - clearAllCrashReports removes all files", "[StabilityService][Crash]")
{
    StabilityServiceTestFixture fixture;
    
    // Write crash logs
    writeTestLog(fixture.crashDir(), "crash_2026-02-01_100000.crash", kSampleCrashLog);
    writeTestLog(fixture.crashDir(), "crash_2026-02-02_100000.crash", kSampleCrashLog);
    
    auto service = fixture.createService();
    
    REQUIRE(service->hasPendingCrashReport());
    REQUIRE(service->getCrashReportFiles().size() == 2);
    
    service->clearAllCrashReports();
    
    REQUIRE_FALSE(service->hasPendingCrashReport());
    REQUIRE(service->getCrashReportFiles().empty());
}

//=============================================================================
// Hang Report Tests
//=============================================================================

TEST_CASE("StabilityService - no pending hang report when directory is empty", "[StabilityService][Hang]")
{
    StabilityServiceTestFixture fixture;
    auto service = fixture.createService();
    
    REQUIRE_FALSE(service->hasPendingHangReport());
    REQUIRE(service->getHangReportFiles().empty());
    REQUIRE_FALSE(service->getLastHangInfo().has_value());
}

TEST_CASE("StabilityService - detects pending hang report", "[StabilityService][Hang]")
{
    StabilityServiceTestFixture fixture;
    
    // Write a hang log before creating service
    writeTestLog(fixture.hangDir(), "hang_2026-02-03_103000.hang", kSampleHangLog);
    
    auto service = fixture.createService();
    
    REQUIRE(service->hasPendingHangReport());
    REQUIRE(service->getHangReportFiles().size() == 1);
}

TEST_CASE("StabilityService - clearAllHangReports removes all files", "[StabilityService][Hang]")
{
    StabilityServiceTestFixture fixture;
    
    // Write hang logs
    writeTestLog(fixture.hangDir(), "hang_2026-02-01_100000.hang", kSampleHangLog);
    writeTestLog(fixture.hangDir(), "hang_2026-02-02_100000.hang", kSampleHangLog);
    
    auto service = fixture.createService();
    
    REQUIRE(service->hasPendingHangReport());
    REQUIRE(service->getHangReportFiles().size() == 2);
    
    service->clearAllHangReports();
    
    REQUIRE_FALSE(service->hasPendingHangReport());
    REQUIRE(service->getHangReportFiles().empty());
}

//=============================================================================
// Heartbeat Tests
//=============================================================================

TEST_CASE("StabilityService - reportHeartbeat does not crash", "[StabilityService][Heartbeat]")
{
    StabilityServiceTestFixture fixture;
    auto service = fixture.createService();
    
    // Simply verify it doesn't throw
    REQUIRE_NOTHROW(service->reportHeartbeat());
    REQUIRE_NOTHROW(service->reportHeartbeat());
    REQUIRE_NOTHROW(service->reportHeartbeat());
}
