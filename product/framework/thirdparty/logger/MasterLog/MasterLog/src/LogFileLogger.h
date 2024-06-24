#ifndef __LogFileLogger_h__
#define __LogFileLogger_h__
#include <string>
#include <fstream>
#include <vector>
#include "LogBaseLogger.h"

namespace LogLogSpace{
    class LogFileLogger final: public LogBaseLogger
    {
    public:
        LogFileLogger(int logLevels, const std::string& logDirPath, const std::string& logBaseName, unsigned int maxKeepDays, unsigned int maxSignleSize, const std::string& loggerName);
        ~LogFileLogger();
    protected:
        void initialize() override;
        void processMessage(const std::string& message) override;
    private:
        std::vector<std::string> getCurrentFileList()const;
        bool readyForLog(unsigned int addedSize);
        void removeOldFiles(const std::vector<std::string>& allFiles);
        void doRollOver(const std::vector<std::string>& allFiles,unsigned int addedSize);
        std::string getCurrentLoggerFilePath() const;
    private:
        int m_logLevels;
        std::string m_logDirPath;
        std::string m_baseFileName;
        unsigned int m_maxKeepDays;
        unsigned int m_maxSingleSize;

	    std::ofstream m_currentFile;
        std::string m_currentFileDate;
        unsigned int m_currentSize;
    };
}
#endif//__LogFileLogger_h__