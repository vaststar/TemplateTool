#ifndef __LogFileLogger_h__
#define __LogFileLogger_h__

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "LogBaseLogger.h"

namespace LogLogSpace {

class LogFileLogger final : public LogBaseLogger
{
public:
    LogFileLogger(
        int logLevels,
        const std::string& logDirPath,
        const std::string& logBaseName,
        unsigned int maxKeepDays,
        unsigned int maxSingleSize,
        const std::string& loggerName);

    ~LogFileLogger() override;

protected:
    void initialize() override;
    void processMessage(const std::string& message) override;

private:
    std::vector<std::string> getCurrentFileList() const;
    bool readyForLog(std::uintmax_t addedSize);

    void removeOldFiles(
        const std::vector<std::string>& allFiles);

    void doRollOver(
        const std::vector<std::string>& allFiles,
        std::uintmax_t addedSize);

    void refreshCurrentFileSize();
    bool openCurrentFile();
    void closeCurrentFile();

    std::string getCurrentLoggerFilePath() const;

private:
    std::string m_logDirPath;
    std::string m_baseFileName;
    unsigned int m_maxKeepDays;
    unsigned int m_maxSingleSize;

    std::ofstream m_currentFile;
    std::string m_currentFileDate;
    std::uintmax_t m_currentSize;
};

}

#endif // __LogFileLogger_h__
