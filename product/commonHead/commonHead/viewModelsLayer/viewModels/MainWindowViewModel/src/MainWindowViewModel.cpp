#include "MainWindowViewModel.h"

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>
#include <commonHead/ViewModelUtils/LogOperationUtils.h>
#include <ucf/Services/InvocationService/IInvocationService.h>
#include <ucf/Services/StabilityService/IStabilityService.h>
#include <ucf/Services/PerformanceService/IPerformanceService.h>

#include <filesystem>
#include <thread>

namespace commonHead::viewModels{
std::shared_ptr<IMainWindowViewModel> IMainWindowViewModel::createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<MainWindowViewModel>(commonHeadFramework);
}

MainWindowViewModel::MainWindowViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : IMainWindowViewModel(commonHeadFramework)
{
    COMMONHEAD_LOG_DEBUG("create MainWindowViewModel");
}

std::string MainWindowViewModel::getViewModelName() const
{
    return "MainWindowViewModel";
}

void MainWindowViewModel::init()
{
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto service = serviceLocator->getInvocationService().lock())
            {
                service->registerCallback(shared_from_this());
            }
        }
    }
}
void MainWindowViewModel::onCommandMessageReceived(const std::string& message)
{
    if (message == "ActivateWindow")
    {
        fireNotification(&IMainWindowViewModelCallback::onActivateMainWindow);
    }
}

void MainWindowViewModel::packApplicationLogs()
{
    COMMONHEAD_LOG_DEBUG("packApplicationLogs called");
    // testCrash();
    // testHang();
    testPerformance();
    
    // Use LogOperationUtils to pack logs - it handles framework/service access internally
    auto result = commonHead::utilities::LogOperationUtils::packLogs(getCommonHeadFramework());
    
    fireNotification(&IMainWindowViewModelCallback::onLogsPackComplete, result.success, result.archivePath);
}

void MainWindowViewModel::testCrash()
{
    COMMONHEAD_LOG_DEBUG("testCrash called");
    
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto stabilityService = serviceLocator->getStabilityService().lock())
            {
                stabilityService->forceCrashForTesting();
            }
        }
    }
}

void MainWindowViewModel::testHang()
{
    COMMONHEAD_LOG_DEBUG("testHang called");
    
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto stabilityService = serviceLocator->getStabilityService().lock())
            {
                stabilityService->forceHangForTesting();
            }
        }
    }
}

void MainWindowViewModel::testPerformance()
{
    COMMONHEAD_LOG_DEBUG("testPerformance called");
    
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto performanceService = serviceLocator->getPerformanceService().lock())
            {
                // Test timing tracking with RAII helper
                {
                    ucf::service::ScopedTiming scopedTiming(serviceLocator->getPerformanceService(), "TestOperation");
                    // Simulate some work
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                
                // Take a snapshot
                auto snapshot = performanceService->takeSnapshot();
                COMMONHEAD_LOG_DEBUG("Snapshot taken - CPU: " + std::to_string(snapshot.cpuUsagePercent) + "%");
                
                // Export report to JSON
                auto jsonReport = performanceService->exportReportAsJson();
                COMMONHEAD_LOG_DEBUG("Performance Report exported");
                
                // Export to file (use system temp directory for cross-platform compatibility)
                std::filesystem::path outputPath = std::filesystem::temp_directory_path() / "performance_report.json";
                performanceService->exportReportToFile(outputPath);
                COMMONHEAD_LOG_DEBUG("Performance report exported to: " + outputPath.string());
            }
        }
    }
}
}