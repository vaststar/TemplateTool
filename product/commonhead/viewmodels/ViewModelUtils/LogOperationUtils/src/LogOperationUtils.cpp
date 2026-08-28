#include <commonhead/viewmodels/ViewModelUtils/LogOperationUtils.h>
#include <commonhead/viewmodels/ViewModelUtils/TimeDisplayUtils.h>

#include <exception>
#include <filesystem>

#include "LoggerDefine.h"
#include <commonhead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonhead/ServiceLocator/IServiceLocator.h>
#include <ucf/services/ClientInfoService/IClientInfoService.h>
#include <ucf/utilities/ArchiveUtils/ArchiveWrapper.h>

namespace commonHead::utilities {

std::string LogOperationUtils::generateTimestampedArchiveName(const std::string& prefix)
{
    const auto timestamp = TimeDisplayUtils::formatCurrentUserTime({
        .localPattern = "%Y%m%d_%H%M%S",
        .utcFallbackPattern = "%Y%m%d_%H%M%SZ",
        .failureText = "unknown"
    });

    return prefix + "_" + timestamp + ".zip";
}

LogOperationResult LogOperationUtils::packLogs(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework,
    const std::string& outputDirectory,
    const std::string& archivePrefix)
{
    LogOperationResult result;
    
    LOG_OPERATION_UTILS_LOG_DEBUG("packLogs called");
    
    // Get log path from ClientInfoService via framework
    auto framework = commonHeadFramework.lock();
    if (!framework)
    {
        result.errorMessage = "CommonHeadFramework is null";
        LOG_OPERATION_UTILS_LOG_ERROR(result.errorMessage);
        return result;
    }
    
    auto serviceLocator = framework->getServiceLocator();
    if (!serviceLocator)
    {
        result.errorMessage = "ServiceLocator is null";
        LOG_OPERATION_UTILS_LOG_ERROR(result.errorMessage);
        return result;
    }
    
    auto clientInfoService = serviceLocator->getClientInfoService().lock();
    if (!clientInfoService)
    {
        result.errorMessage = "ClientInfoService is null";
        LOG_OPERATION_UTILS_LOG_ERROR(result.errorMessage);
        return result;
    }
    
    std::string logDirectoryPath = clientInfoService->getAppLogStoragePath();
    LOG_OPERATION_UTILS_LOG_DEBUG("Log directory path: " << logDirectoryPath);
    
    // Validate log directory exists
    if (logDirectoryPath.empty())
    {
        result.errorMessage = "Log directory path is empty";
        LOG_OPERATION_UTILS_LOG_ERROR(result.errorMessage);
        return result;
    }
    
    if (!std::filesystem::exists(logDirectoryPath))
    {
        result.errorMessage = "Log directory does not exist: " + logDirectoryPath;
        LOG_OPERATION_UTILS_LOG_ERROR(result.errorMessage);
        return result;
    }
    
    // Determine output directory
    std::filesystem::path logDir(logDirectoryPath);
    std::filesystem::path outDir;
    
    if (outputDirectory.empty())
    {
        outDir = logDir.parent_path();
    }
    else
    {
        outDir = std::filesystem::path(outputDirectory);
        if (!std::filesystem::exists(outDir))
        {
            try
            {
                std::filesystem::create_directories(outDir);
            }
            catch (const std::exception& e)
            {
                result.errorMessage = std::string("Failed to create output directory: ") + e.what();
                LOG_OPERATION_UTILS_LOG_ERROR(result.errorMessage);
                return result;
            }
        }
    }
    
    // Generate archive path
    std::string archiveName = generateTimestampedArchiveName(archivePrefix);
    std::filesystem::path fullArchivePath = outDir / archiveName;
    result.archivePath = fullArchivePath.string();
    
    LOG_OPERATION_UTILS_LOG_DEBUG("Packing logs from: " << logDirectoryPath << " to: " << result.archivePath);
    
    // Create archive
    ucf::utilities::ArchiveWrapper archiveWrapper;
    auto archiveResult = archiveWrapper.createFromDirectory(result.archivePath, logDirectoryPath, false);
    result.success = (archiveResult == ucf::utilities::ArchiveError::Success);
    
    if (result.success)
    {
        LOG_OPERATION_UTILS_LOG_DEBUG("Successfully packed logs to: " << result.archivePath);
    }
    else
    {
        result.errorMessage = "Failed to create archive";
        LOG_OPERATION_UTILS_LOG_ERROR(result.errorMessage);
    }
    
    return result;
}

} // namespace commonHead::utilities
