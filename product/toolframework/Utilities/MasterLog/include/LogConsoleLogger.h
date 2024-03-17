#ifndef __LogConsoleLogger_h__
#define __LogConsoleLogger_h__
#include <string>
#include "LogBaseLogger.h"
namespace LogLogSpace{
    class LogConsoleLogger final: public LogBaseLogger
    {
    public:
        explicit LogConsoleLogger(int loggerLevels, const std::string& loggerName);
        ~LogConsoleLogger()=default;
    protected:
        void initialize() override;
        void processMessage(const std::string& message) override;
    };
}
#endif//__LogConsoleLogger_h__