#include "LogConsoleLogger.h"
#include <iostream>

namespace LogLogSpace{
    LogConsoleLogger::LogConsoleLogger(int loggerLevels, const std::string& loggerName)
        :LogBaseLogger(loggerLevels, loggerName)
    {

    }
    
    void LogConsoleLogger::initialize()
    {
        
    }

    void LogConsoleLogger::processMessage(const std::queue<std::string>& messages) 
    {
        std::queue<std::string> logMessages = messages;
        while(!logMessages.empty())
        {
            std::cout<<logMessages.front()<<std::endl;
            logMessages.pop();
        }
    }
}
