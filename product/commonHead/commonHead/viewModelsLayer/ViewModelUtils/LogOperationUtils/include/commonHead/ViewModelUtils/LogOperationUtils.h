#pragma once

#include <string>
#include <memory>
#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

namespace commonHead {
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWptr = std::weak_ptr<ICommonHeadFramework>;
}

namespace commonHead::utilities {

/**
 * @brief Result of log operation
 */
struct COMMONHEAD_EXPORT LogOperationResult {
    bool success{false};
    std::string archivePath;
    std::string errorMessage;
};

/**
 * @brief LogOperationUtils - Utility class for application log operations
 * 
 * This class provides static methods to pack, manage application logs.
 * It uses CommonHeadFramework to access services like ClientInfoService
 * for log paths.
 */
class COMMONHEAD_EXPORT LogOperationUtils final {
public:
    LogOperationUtils() = delete;
    
    /**
     * @brief Pack application logs into a ZIP archive
     * @param commonHeadFramework Framework to access services (ClientInfoService for log path)
     * @param outputDirectory Optional output directory for the archive (default: parent of log directory)
     * @param archivePrefix Prefix for the archive filename (default: "logs")
     * @return LogOperationResult containing success status and archive path
     */
    static LogOperationResult packLogs(
        commonHead::ICommonHeadFrameworkWptr commonHeadFramework,
        const std::string& outputDirectory = "",
        const std::string& archivePrefix = "logs"
    );
    
    /**
     * @brief Generate a timestamped archive filename
     * @param prefix Prefix for the filename
     * @return Filename in format: {prefix}_{YYYYMMDD_HHMMSS}.zip
     */
    static std::string generateTimestampedArchiveName(const std::string& prefix = "logs");
};

} // namespace commonHead::utilities
