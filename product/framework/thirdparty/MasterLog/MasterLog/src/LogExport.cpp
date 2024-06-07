#include <MasterLog/LogExport.h>
#include "LogControl.h"
#include <algorithm>

namespace MasterLogUtil{
void WriteLog(const std::string& logTag, LogLogSpace::LogLevel logLevel, const std::string& filePath, 
                          int lineNumber,const std::string& functionName, const std::string& logMessage, const std::string& loggerName)
{
    LogLogSpace::LogControl::getInstance()->writeLog(logTag,static_cast<int>(logLevel), filePath, lineNumber, functionName, logMessage, loggerName);
}

void InitLogger(const std::vector<std::shared_ptr<LogLogSpace::LoggerBaseConfigure>>& configure)
{
    std::for_each(configure.cbegin(), configure.cend(), [](const std::shared_ptr<LogLogSpace::LoggerBaseConfigure>& config){
        if (auto consoleConfig = std::dynamic_pointer_cast<LogLogSpace::LoggerConsoleConfigure>(config))
        {
            LogLogSpace::LogControl::getInstance()->initConsoleLogger(consoleConfig->loggerLevels, consoleConfig->loggerName); 
        }
        else if (auto fileConfig = std::dynamic_pointer_cast<LogLogSpace::LoggerFileConfigure>(config))
        {
            LogLogSpace::LogControl::getInstance()->initFileLogger(fileConfig->loggerLevels, fileConfig->loggerDirPath, fileConfig->loggerBaseName, fileConfig->maxKeepDays, fileConfig->maxSingleFileSize, fileConfig->loggerName); 
        }
    });
}

void StopLogger()
{
    LogLogSpace::LogControl::getInstance()->stopLogger();
}
}
