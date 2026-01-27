#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <memory>

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

namespace ucf::utilities {

//============================================
// 日志级别
//============================================
enum class LogLevel {
    Debug = 1 << 0,
    Info  = 1 << 1,
    Warn  = 1 << 2,
    Error = 1 << 3,
    Fatal = 1 << 4
};

inline constexpr int kAllLogLevels = 
    static_cast<int>(LogLevel::Debug) | static_cast<int>(LogLevel::Info) |
    static_cast<int>(LogLevel::Warn) | static_cast<int>(LogLevel::Error) |
    static_cast<int>(LogLevel::Fatal);

inline constexpr int kExcludeDebugLogLevels = 
    static_cast<int>(LogLevel::Info) | static_cast<int>(LogLevel::Warn) |
    static_cast<int>(LogLevel::Error) | static_cast<int>(LogLevel::Fatal);

//============================================
// 常用 Logger 名称
//============================================
inline constexpr const char* kAppLoggerName = "APP";
inline constexpr const char* kConsoleLoggerName = "ConsoleLogger";

//============================================
// Logger 配置
//============================================
struct LoggerConfig {
    virtual ~LoggerConfig() = default;
    int mLogLevels{kAllLogLevels};
    std::string mLoggerName;
};

struct ConsoleLoggerConfig : public LoggerConfig {
    explicit ConsoleLoggerConfig(int levels, const std::string& name) {
        mLogLevels = levels;
        mLoggerName = name;
    }
};

struct FileLoggerConfig : public LoggerConfig {
    FileLoggerConfig(int levels, const std::string& dirPath, 
                     const std::string& baseName, 
                     unsigned int keepDays, unsigned int maxFileSize,
                     const std::string& name) 
        : mLoggerDirPath(dirPath)
        , mLoggerBaseName(baseName)
        , mMaxKeepDays(keepDays)
        , mMaxSingleFileSize(maxFileSize)
    {
        mLogLevels = levels;
        mLoggerName = name;
    }
    
    std::string mLoggerDirPath;
    std::string mLoggerBaseName;
    unsigned int mMaxKeepDays{180};
    unsigned int mMaxSingleFileSize{20 * 1024 * 1024};
};

//============================================
// 日志函数声明
//============================================
Utilities_EXPORT void writeLog(const std::string& tag, LogLevel level,
                                const std::string& filePath, int lineNumber,
                                const std::string& functionName,
                                const std::string& message,
                                const std::string& loggerName);

Utilities_EXPORT void initLogger(const std::vector<std::shared_ptr<LoggerConfig>>& configs);
Utilities_EXPORT void stopLogger();

} // namespace ucf::utilities

//============================================
// 日志宏（完全隔离底层实现）
//============================================

#define UCF_LOG_DEBUG(tag, message, loggerName) \
    { std::ostringstream _oss; _oss << message; \
      ucf::utilities::writeLog(tag, ucf::utilities::LogLevel::Debug, \
                                __FILE__, __LINE__, __FUNCTION__, _oss.str(), loggerName); }

#define UCF_LOG_INFO(tag, message, loggerName) \
    { std::ostringstream _oss; _oss << message; \
      ucf::utilities::writeLog(tag, ucf::utilities::LogLevel::Info, \
                                __FILE__, __LINE__, __FUNCTION__, _oss.str(), loggerName); }

#define UCF_LOG_WARN(tag, message, loggerName) \
    { std::ostringstream _oss; _oss << message; \
      ucf::utilities::writeLog(tag, ucf::utilities::LogLevel::Warn, \
                                __FILE__, __LINE__, __FUNCTION__, _oss.str(), loggerName); }

#define UCF_LOG_ERROR(tag, message, loggerName) \
    { std::ostringstream _oss; _oss << message; \
      ucf::utilities::writeLog(tag, ucf::utilities::LogLevel::Error, \
                                __FILE__, __LINE__, __FUNCTION__, _oss.str(), loggerName); }

#define UCF_LOG_FATAL(tag, message, loggerName) \
    { std::ostringstream _oss; _oss << message; \
      ucf::utilities::writeLog(tag, ucf::utilities::LogLevel::Fatal, \
                                __FILE__, __LINE__, __FUNCTION__, _oss.str(), loggerName); }

//============================================
// 初始化/停止宏
//============================================

#define UCF_LOG_INIT(configs)  ucf::utilities::initLogger(configs)
#define UCF_LOG_STOP()         ucf::utilities::stopLogger()
