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

    void LogConsoleLogger::processMessage(const std::string& message) 
    {
        std::cout<<message<<std::endl;
    }
}
