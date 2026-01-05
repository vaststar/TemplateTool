#ifndef __LogExport_h__
#define __LogExport_h__

#include <memory>
#include <string>
#include <ostream>
#include <sstream>
#include <vector>
#include <MasterLog/LogDefine.h>


#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable:4251)//for template class member with dll-export, it will report warning, so, just disable it
    #if defined(MASTERLOG_LIB) // for static library
        #define LOG_LIB_API
    #elif defined(MASTERLOG_DLL) // for dynamic library
        #define LOG_LIB_API __declspec(dllexport)
    #else
        #define LOG_LIB_API __declspec(dllimport)
    #endif
#else
    #if defined(MASTERLOG_DLL)
        #define LOG_LIB_API __attribute__((visibility("default")))
    #else
        #define LOG_LIB_API
    #endif
#endif

namespace MasterLogUtil{
inline constexpr int ALL_LOG_LEVEL = LogLogSpace::LogLevel::LOG_DEBUG|LogLogSpace::LogLevel::LOG_INFO|LogLogSpace::LogLevel::LOG_WARN|LogLogSpace::LogLevel::LOG_ERROR|LogLogSpace::LogLevel::LOG_FATAL;
inline constexpr int EXCLUDE_DEBUG_LOG_LEVEL = LogLogSpace::LogLevel::LOG_INFO|LogLogSpace::LogLevel::LOG_WARN|LogLogSpace::LogLevel::LOG_ERROR|LogLogSpace::LogLevel::LOG_FATAL;
inline constexpr auto Console_Logger_Name = "ConsoleLogger";
inline constexpr auto Default_File_Logger_Name = "DefaultFileLogger";

LOG_LIB_API void WriteLog(const std::string& logTag, LogLogSpace::LogLevel logLevel, const std::string& filePath, 
                          int lineNumber, const std::string& functionName, const std::string& logMessage, const std::string& loggerName);
LOG_LIB_API void InitLogger(const std::vector<std::shared_ptr<LogLogSpace::LoggerBaseConfigure>>& configure);
LOG_LIB_API void StopLogger();
}

#define LOG_DEBUG(logTag,logMessage,loggerName) {std::ostringstream _oss; _oss << logMessage;MasterLogUtil::WriteLog(logTag,LogLogSpace::LogLevel::LOG_DEBUG,__FILE__,__LINE__,__FUNCTION__,_oss.str(),loggerName);}
#define LOG_INFO(logTag,logMessage,loggerName) {std::ostringstream _oss; _oss << logMessage;MasterLogUtil::WriteLog(logTag,LogLogSpace::LogLevel::LOG_INFO,__FILE__,__LINE__,__FUNCTION__,_oss.str(),loggerName);}
#define LOG_WARN(logTag,logMessage,loggerName) {std::ostringstream _oss; _oss << logMessage;MasterLogUtil::WriteLog(logTag,LogLogSpace::LogLevel::LOG_WARN,__FILE__,__LINE__,__FUNCTION__,_oss.str(),loggerName);}
#define LOG_ERROR(logTag,logMessage,loggerName) {std::ostringstream _oss; _oss << logMessage;MasterLogUtil::WriteLog(logTag,LogLogSpace::LogLevel::LOG_ERROR,__FILE__,__LINE__,__FUNCTION__,_oss.str(),loggerName);}
#define LOG_FATAL(logTag,logMessage,loggerName) {std::ostringstream _oss; _oss << logMessage;MasterLogUtil::WriteLog(logTag,LogLogSpace::LogLevel::LOG_FATAL,__FILE__,__LINE__,__FUNCTION__,_oss.str(),loggerName);}
#endif//__LogExport_h__