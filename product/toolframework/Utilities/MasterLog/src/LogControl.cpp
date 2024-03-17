
#include "LogBaseLogger.h"
#include "LogControl.h"

#include <mutex>
#include <thread>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

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

std::string LogControl::getCurrentFormatedTime() const
{
    std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
	time_t raw_time = std::chrono::system_clock::to_time_t(tp);
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
	std::string milliseconds_str = std::to_string(ms.count() % 1000);
	if (milliseconds_str.length() < 3) {
		milliseconds_str = std::string(3 - milliseconds_str.length(), '0') + milliseconds_str;
	}
	std::stringstream ss;
	ss << std::put_time(std::localtime(&raw_time), "%Y-%m-%d %H:%M:%S,")<<milliseconds_str;
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
    
    std::stringstream result;
    result << getCurrentFormatedTime() << " " << getLogLevelString(logLevel) << " [" << std::this_thread::get_id() << "] ["<< fileString << "] ["
           << logTag << "] [" << functionName << "] - " << logMessage <<"\n";
    return result.str();
}
}
