#include "LogBaseLogger.h"

#include <format>
#include <functional>

#include "MasterLogConfig.h"

namespace LogLogSpace{
    LogBaseLogger::LogBaseLogger(int loglevels, const std::string& loggerName)
        :m_isInExit(false)
        ,m_loggerLevels(loglevels)
        ,m_loggerName(loggerName)
    {

    }

    LogBaseLogger::~LogBaseLogger()
    {
        stopLog();
    }

    std::string LogBaseLogger::getLoggerName() const
    {
        return m_loggerName;
    }

    void LogBaseLogger::startLog()
    {
        std::call_once(start_flag, [this]() {
            initialize();
            m_workThread = std::make_unique<std::thread>(std::bind(&LogBaseLogger::doWorkFunction,this));
        });
    }

    void LogBaseLogger::stopLog()
    {
        std::call_once(stop_flag, [this]() {
            std::queue<std::string> lastMessages;
            {
                std::scoped_lock<std::mutex> loc(m_dataMutex);
                m_isInExit = true;
                std::swap(lastMessages, m_logMessages);
            }
            //deal the last messages
            dealMessages(lastMessages);

            //join the task
            m_condition.notify_one();
            if(m_workThread && m_workThread->joinable())
            {
                m_workThread->join();
            }
        });
    }

    void LogBaseLogger::appendLog(int loggerLevel, const std::string& message)
    {
        {
            std::scoped_lock<std::mutex> loc(m_dataMutex);
            if(m_isInExit || !(loggerLevel & m_loggerLevels))
            {
                return;
            }
            m_logMessages.push(message);
        }
        m_condition.notify_one();
    }

    void LogBaseLogger::doWorkFunction()
    {
        while (true)
        {
            std::queue<std::string> currentLogs;
            {
                std::unique_lock<std::mutex> guard(m_dataMutex);
                m_condition.wait(guard,[this](){return !m_logMessages.empty() || m_isInExit;});
                if(m_isInExit)
                {
                    return;
                }
                std::swap(currentLogs,m_logMessages);
            }
            dealMessages(currentLogs);
        }
    }
    
    void LogBaseLogger::dealMessages(const std::queue<std::string>& logMessages)
    {
        if (!logMessages.empty())
        {
            std::queue<std::string> messages = logMessages;
            std::scoped_lock<std::mutex> loc(m_witeMutex);
            while(!messages.empty())
            {
                processMessage(messages.front());
                messages.pop();
            }
        }
    }
}
