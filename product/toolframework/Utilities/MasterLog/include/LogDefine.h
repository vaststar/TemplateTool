#ifndef __LogDefine_h__
#define __LogDefine_h__

namespace LogLogSpace{
    enum class LogLevel{
        LOG_DEBUG = 1 << 0,
        LOG_INFO = 1 << 1,
        LOG_WARN = 1 << 2,
        LOG_ERROR = 1 << 3,
        LOG_FATAL = 1 << 4
    };

    inline constexpr int operator|(LogLevel lhs, LogLevel rhs)
    {
        return static_cast<int>(lhs) | static_cast<int>(rhs);
    }
    inline constexpr int operator|(int lhs, LogLevel rhs)
    {
        return lhs | static_cast<int>(rhs);
    }
    inline constexpr int operator|(LogLevel lhs, int rhs)
    {
        return static_cast<int>(lhs) | static_cast<int>(rhs);
    }
    inline constexpr bool operator&(LogLevel lhs, LogLevel rhs)
    {
        return 0 != (static_cast<int>(lhs) & static_cast<int>(rhs));
    }  
    inline constexpr bool operator&(int lhs, LogLevel rhs)
    {
        return 0 != (lhs & static_cast<int>(rhs));
    } 
    inline constexpr bool operator&(LogLevel lhs, int rhs)
    {
        return 0 != (static_cast<int>(lhs) & rhs);
    } 

    struct LoggerBaseConfigure{
        virtual ~LoggerBaseConfigure() = default;
        explicit LoggerBaseConfigure(int _loggerLevels, const std::string& _loggerName)
            : loggerLevels(_loggerLevels)
            , loggerName(_loggerName)
        {}
        int loggerLevels{LogLogSpace::LogLevel::LOG_DEBUG|LogLogSpace::LogLevel::LOG_INFO|LogLogSpace::LogLevel::LOG_WARN|LogLogSpace::LogLevel::LOG_ERROR|LogLogSpace::LogLevel::LOG_FATAL};//use LogLevel &
        std::string loggerName;
    };

    struct LoggerConsoleConfigure: public LoggerBaseConfigure{
        explicit LoggerConsoleConfigure(int _loggerLevels, const std::string& _loggerName): LoggerBaseConfigure(_loggerLevels, _loggerName)
        {}
    };

    struct LoggerFileConfigure: public LoggerBaseConfigure{
        LoggerFileConfigure(int _loggerLevels, const std::string& _loggerDirPath, const std::string& _loggerBaseName, unsigned int _maxKeepDays, unsigned int _maxSingleFileSize, const std::string& _loggerName)
            : LoggerBaseConfigure(_loggerLevels, _loggerName)
            , loggerDirPath(_loggerDirPath)
            , loggerBaseName(_loggerBaseName)
            , maxKeepDays(_maxKeepDays)
            , maxSingleFileSize(_maxSingleFileSize)
        {}
        std::string loggerDirPath;
        std::string loggerBaseName;
        unsigned int maxKeepDays{ 180 };
        unsigned int maxSingleFileSize{ 20*1024*1024 };//bit
    };
}
#endif // __LogDefine_h__