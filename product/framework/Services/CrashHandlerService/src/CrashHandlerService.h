#pragma once

#include <ucf/Services/CrashHandlerService/ICrashHandlerService.h>
#include <memory>

namespace ucf::framework {
    class ICoreFramework;
}

namespace ucf::service {

class CrashHandlerManager;

class CrashHandlerService : public ICrashHandlerService
{
public:
    explicit CrashHandlerService(std::shared_ptr<ucf::framework::ICoreFramework> coreFramework);
    ~CrashHandlerService() override;

    // IService
    [[nodiscard]] std::string getServiceName() const override { return "CrashHandlerService"; }

    // ICrashHandlerService - Status
    [[nodiscard]] bool isInstalled() const override;

    // ICrashHandlerService - Crash Report
    [[nodiscard]] bool hasPendingCrashReport() const override;
    [[nodiscard]] std::optional<CrashInfo> getLastCrashInfo() const override;
    [[nodiscard]] std::vector<std::filesystem::path> getCrashReportFiles() const override;
    void clearPendingCrashReport() override;
    void clearAllCrashReports() override;

    // ICrashHandlerService - Testing
    void forceCrashForTesting() override;

protected:
    void initService() override;

private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};

} // namespace ucf::service
