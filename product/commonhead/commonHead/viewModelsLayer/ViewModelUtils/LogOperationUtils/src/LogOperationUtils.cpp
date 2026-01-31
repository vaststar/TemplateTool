#include <commonHead/ViewModelUtils/LogOperationUtils.h>

#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>

#include "LoggerDefine.h"
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>
#include <ucf/Utilities/ArchiveUtils/ArchiveWrapper.h>

namespace commonHead::utilities {

std::string LogOperationUtils::generateTimestampedArchiveName(const std::string& prefix)
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << prefix << "_";
    ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
    ss << ".zip";
    return ss.str();
}

LogOperationResult LogOperationUtils::packLogs(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework,
    const std::string& outputDirectory,
    const std::string& archivePrefix)
{
    LogOperationResult result;
    
    VIEWMODELUTILS_LOG_DEBUG("packLogs called");
    
    // Get log path from ClientInfoService via framework
    auto framework = commonHeadFramework.lock();
    if (!framework)
    {
        result.errorMessage = "CommonHeadFramework is null";
        VIEWMODELUTILS_LOG_ERROR(result.errorMessage);
        return result;
    }
    
    auto serviceLocator = framework->getServiceLocator();
    if (!serviceLocator)
    {
        result.errorMessage = "ServiceLocator is null";
        VIEWMODELUTILS_LOG_ERROR(result.errorMessage);
        return result;
    }
    
    auto clientInfoService = serviceLocator->getClientInfoService().lock();
    if (!clientInfoService)
    {
        result.errorMessage = "ClientInfoService is null";
        VIEWMODELUTILS_LOG_ERROR(result.errorMessage);
        return result;
    }
    
    std::string logDirectoryPath = clientInfoService->getAppLogStoragePath();
    VIEWMODELUTILS_LOG_DEBUG("Log directory path: " << logDirectoryPath);
    
    // Validate log directory exists
    if (logDirectoryPath.empty())
    {
        result.errorMessage = "Log directory path is empty";
        VIEWMODELUTILS_LOG_ERROR(result.errorMessage);
        return result;
    }
    
    if (!std::filesystem::exists(logDirectoryPath))
    {
        result.errorMessage = "Log directory does not exist: " + logDirectoryPath;
        VIEWMODELUTILS_LOG_ERROR(result.errorMessage);
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
                VIEWMODELUTILS_LOG_ERROR(result.errorMessage);
                return result;
            }
        }
    }
    
    // Generate archive path
    std::string archiveName = generateTimestampedArchiveName(archivePrefix);
    std::filesystem::path fullArchivePath = outDir / archiveName;
    result.archivePath = fullArchivePath.string();
    
    VIEWMODELUTILS_LOG_DEBUG("Packing logs from: " << logDirectoryPath << " to: " << result.archivePath);
    
    // Create archive
    ucf::utilities::ArchiveWrapper archiveWrapper;
    auto archiveResult = archiveWrapper.createFromDirectory(result.archivePath, logDirectoryPath, false);
    result.success = (archiveResult == ucf::utilities::ArchiveError::Success);
    
    if (result.success)
    {
        VIEWMODELUTILS_LOG_DEBUG("Successfully packed logs to: " << result.archivePath);
    }
    else
    {
        result.errorMessage = "Failed to create archive";
        VIEWMODELUTILS_LOG_ERROR(result.errorMessage);
    }
    
    return result;
}

} // namespace commonHead::utilities
