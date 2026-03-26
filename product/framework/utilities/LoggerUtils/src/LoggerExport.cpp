#include <ucf/Utilities/LoggerUtils/LoggerExport.h>
#include <MasterLog/LogExport.h>

namespace ucf::utilities {

void writeLog(const std::string& tag, LogLevel level,
              const std::string& filePath, int lineNumber,
              const std::string& functionName,
              const std::string& message,
              const std::string& loggerName)
{
    // ucf::utilities::LogLevel → LogLogSpace::LogLevel
    LogLogSpace::LogLevel masterLevel;
    switch (level) {
        case LogLevel::Debug: masterLevel = LogLogSpace::LogLevel::LOG_DEBUG; break;
        case LogLevel::Info:  masterLevel = LogLogSpace::LogLevel::LOG_INFO; break;
        case LogLevel::Warn:  masterLevel = LogLogSpace::LogLevel::LOG_WARN; break;
        case LogLevel::Error: masterLevel = LogLogSpace::LogLevel::LOG_ERROR; break;
        case LogLevel::Fatal: masterLevel = LogLogSpace::LogLevel::LOG_FATAL; break;
        default:              masterLevel = LogLogSpace::LogLevel::LOG_INFO; break;
    }
    
    MasterLogUtil::WriteLog(tag, masterLevel, filePath, lineNumber, 
                            functionName, message, loggerName);
}

void initLogger(const std::vector<std::shared_ptr<LoggerConfig>>& configs)
{
    std::vector<std::shared_ptr<LogLogSpace::LoggerBaseConfigure>> masterConfigs;
    
    for (const auto& config : configs) {
        if (auto consoleConfig = std::dynamic_pointer_cast<ConsoleLoggerConfig>(config)) {
            masterConfigs.push_back(std::make_shared<LogLogSpace::LoggerConsoleConfigure>(
                consoleConfig->mLogLevels, consoleConfig->mLoggerName));
        } else if (auto fileConfig = std::dynamic_pointer_cast<FileLoggerConfig>(config)) {
            masterConfigs.push_back(std::make_shared<LogLogSpace::LoggerFileConfigure>(
                fileConfig->mLogLevels, 
                fileConfig->mLoggerDirPath, 
                fileConfig->mLoggerBaseName,
                fileConfig->mMaxKeepDays, 
                fileConfig->mMaxSingleFileSize, 
                fileConfig->mLoggerName));
        }
    }
    
    MasterLogUtil::InitLogger(masterConfigs);
}

void stopLogger()
{
    MasterLogUtil::StopLogger();
}

} // namespace ucf::utilities
