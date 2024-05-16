#ifndef __LogControl_h__
#define __LogControl_h__

#include <memory>
#include <string>
#include <list>
#include <mutex>

class LogBaseLogger;

namespace LogLogSpace{
    class LogControl final: std::enable_shared_from_this<LogControl>
    {
    public:
        static std::shared_ptr<LogControl> getInstance();

        void initConsoleLogger(int logLevels, const std::string& loggerName);
        void initFileLogger( int logLevels, const std::string& logDirPath, const std::string& logBaseName, unsigned int maxKeepDays, unsigned int maxSingleFileSize, const std::string& loggerName);
        void writeLog(const std::string& logTag, int logLevel, const std::string& filePath, 
                      int lineNumber,const std::string& functionName, const std::string& logMessage, const std::string& loggerName);
    private:
        std::mutex m_loggerMutex;
        std::list<std::unique_ptr<LogBaseLogger>> m_currentLogger;
    private:
        void createLoggers();
        std::string formatMessage(const std::string& logTag, int logLevel, const std::string& filePath, 
                                  int lineNumber,const std::string& functionName, const std::string& logMessage)const;
        std::string getCurrentFormatedTime() const;
        std::string getLogLevelString(int logLevel) const;
        std::string getCurrentThreadId() const;
    private:
        LogControl() = default;
        static std::shared_ptr<LogControl> _instance;
    public:
        LogControl(const LogControl& rhs) = delete;
        LogControl& operator=(const LogControl& rhs) = delete;
        LogControl(LogControl&& rhs) = delete;
        LogControl& operator=(LogControl&& rhs) = delete;
        ~LogControl();
    };
}
#endif// __LogControl_h__