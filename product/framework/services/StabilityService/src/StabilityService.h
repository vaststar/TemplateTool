#pragma once

#include <ucf/Services/StabilityService/IStabilityService.h>
#include <memory>

namespace ucf::framework {
    class ICoreFramework;
}

namespace ucf::service {

class SERVICE_EXPORT StabilityService : public IStabilityService
{
public:
    explicit StabilityService(std::shared_ptr<ucf::framework::ICoreFramework> coreFramework);
    ~StabilityService() override;

    // IService
    [[nodiscard]] std::string getServiceName() const override { return "StabilityService"; }

    // IStabilityService - Status
    [[nodiscard]] bool isCrashHandlerInstalled() const override;

    // IStabilityService - Crash Report
    [[nodiscard]] bool hasPendingCrashReport() const override;
    [[nodiscard]] std::optional<CrashInfo> getLastCrashInfo() const override;
    [[nodiscard]] std::vector<std::filesystem::path> getCrashReportFiles() const override;
    void clearPendingCrashReport() override;
    void clearAllCrashReports() override;

    // IStabilityService - Hang Detection
    void reportHeartbeat() override;
    [[nodiscard]] bool isHangDetectionEnabled() const override;
    [[nodiscard]] bool hasPendingHangReport() const override;
    [[nodiscard]] std::optional<HangInfo> getLastHangInfo() const override;
    [[nodiscard]] std::vector<std::filesystem::path> getHangReportFiles() const override;
    void clearPendingHangReport() override;
    void clearAllHangReports() override;

    // IStabilityService - Testing
    void forceCrashForTesting() override;
    void forceHangForTesting() override;

protected:
    void initService() override;

private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};

} // namespace ucf::service
