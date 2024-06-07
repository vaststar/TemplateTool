
#include "LogBaseLogger.h"
#include "LogControl.h"

#include <mutex>
#include <thread>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <format>

#include "LogConsoleLogger.h"
#include "LogFileLogger.h"

namespace LogLogSpace{
std::shared_ptr<LogControl> LogControl::_instance = nullptr;
std::shared_ptr<LogControl> LogControl::getInstance()
{
    static std::once_flag s_flag;
    std::call_once(s_flag, [&]() {
        _instance.reset(new LogControl());
    });
    return _instance;
}

LogControl::~LogControl()
{
    std::scoped_lock<std::mutex> lo(m_loggerMutex);
    m_currentLogger.clear();
}

void LogControl::initConsoleLogger(int logLevels, const std::string& loggerName)
{
    static std::once_flag init_console_flag;
    std::call_once(init_console_flag, [logLevels, loggerName, this]() {
        std::scoped_lock<std::mutex> lo(m_loggerMutex);
        m_currentLogger.emplace_back(std::make_unique<LogConsoleLogger>(logLevels, loggerName));
        m_currentLogger.back()->startLog();
    });
}

void LogControl::initFileLogger( int logLevels, const std::string& logDirPath, const std::string& logBaseName, unsigned int maxKeepDays, unsigned int maxSingleFileSize, const std::string& loggerName)
{
    std::scoped_lock<std::mutex> lo(m_loggerMutex);
    if(std::none_of(m_currentLogger.cbegin(), m_currentLogger.cend(), [loggerName](const auto& logger){
        return logger->getLoggerName() == loggerName;
    }))
    {
        m_currentLogger.emplace_back(std::make_unique<LogFileLogger>(logLevels, logDirPath, logBaseName, maxKeepDays, maxSingleFileSize, loggerName));
        m_currentLogger.back()->startLog();
    }
}

void LogControl::writeLog(const std::string& logTag, int logLevel, const std::string& filePath, 
              int lineNumber,const std::string& functionName, const std::string& logMessage, const std::string& loggerName)
{
    std::string messageLog = formatMessage(logTag,logLevel, filePath, lineNumber, functionName, logMessage);
    std::scoped_lock<std::mutex> lo(m_loggerMutex);
    std::for_each(m_currentLogger.begin(),m_currentLogger.end(),[logLevel, &messageLog, loggerName](std::unique_ptr<LogBaseLogger>& logger){
        if (logger->getLoggerName() == loggerName)
        {
            logger->appendLog(logLevel,messageLog);
        }
    });
}

void LogControl::stopLogger()
{
    std::scoped_lock<std::mutex> lo(m_loggerMutex);
    std::for_each(m_currentLogger.begin(),m_currentLogger.end(),[](std::unique_ptr<LogBaseLogger>& logger){
        logger->stopLog();
    });
    m_currentLogger.clear();
}

std::string LogControl::getCurrentFormatedTime() const
{
    const auto start = std::chrono::utc_clock::now();
    return std::format("{:%Y-%m-%dT%H:%M:%SZ}",start);
}

std::string LogControl::getCurrentThreadId() const
{
    std::stringstream ss;
    ss << "0x" << std::hex << std::this_thread::get_id();
    return ss.str();
}

std::string LogControl::getLogLevelString(int logLevel) const
{
    switch (logLevel)
    {
    case 1<<0:
        return "DEBUG";
    case 1<<1:
        return "INFO ";
    case 1<<2:
        return "WARN ";
    case 1<<3:
        return "ERROR";
    case 1<<4:
        return "FATAL";
    default:
        return "UNKNOWN";
    }
}

std::string LogControl::formatMessage(const std::string& logTag, int logLevel, const std::string& filePath, 
                          int lineNumber,const std::string& functionName, const std::string& logMessage)const
{
    //filepath+linenumber
    std::string fileString = filePath + "(" + std::to_string(lineNumber) + ")";
    if(fileString.length() > 40)
    {
        fileString = fileString.substr(fileString.length()-40,40);
    }
    else if(fileString.length() < 40)
    {
        fileString = std::string(40 - fileString.length(),' ') + fileString;
    }
    
    return std::format("{} {} [{}] [{}] [{}] [{}] {}\n", getCurrentFormatedTime(), getLogLevelString(logLevel), getCurrentThreadId(), fileString, logTag, functionName, logMessage);
}
}
