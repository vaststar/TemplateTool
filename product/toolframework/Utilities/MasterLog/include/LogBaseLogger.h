#ifndef __LogBaseLogger_h__
#define __LogBaseLogger_h__

#include <string>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace LogLogSpace{
    class LogBaseLogger
    {
    public:
        explicit LogBaseLogger(int loglevels, const std::string& loggerName);
        virtual ~LogBaseLogger();
        void appendLog(int loggerLevel, const std::string& message);
        void startLog();
        std::string getLoggerName() const;
    protected:
        virtual void initialize() = 0;
        virtual void processMessage(const std::string& message) = 0;
    private:
        void doWorkFunction();
        void writeInitLog();
    private:
        int m_loggerLevels;
        std::string m_loggerName;
        std::condition_variable m_condition;
        std::mutex m_dataMutex;
        bool m_isInExit;
        std::once_flag start_flag;
        std::queue<std::string> m_logMessages;
        std::unique_ptr<std::thread> m_workThread;
        std::mutex m_witeMutex;
    };
}
#endif//__LogBaseLogger_h__