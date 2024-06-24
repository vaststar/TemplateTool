#include "LogFileLogger.h"

#include <regex>
#include <chrono>
#include <sstream>
#include <iostream>
#include <filesystem>

namespace LogLogSpace{
    LogFileLogger::LogFileLogger(int logLevels, const std::string& logDirPath, const std::string& logBaseName, unsigned int maxKeepDays, unsigned int maxSignleSize, const std::string& loggerName)
        :LogBaseLogger(logLevels, loggerName)
        ,m_logDirPath(logDirPath)
        ,m_baseFileName(logBaseName)
        ,m_logLevels(logLevels)
        ,m_maxKeepDays(maxKeepDays)
        ,m_maxSingleSize(maxSignleSize)
        ,m_currentSize(0)
    {
    }

    LogFileLogger::~LogFileLogger()
    {
        if(m_currentFile.is_open())
        {
            m_currentFile.flush();
            m_currentFile.close();
        }
    }

    void LogFileLogger::initialize()
    {
        std::filesystem::create_directories(m_logDirPath);
    }

    void LogFileLogger::processMessage(const std::string& message) 
    {
        if(readyForLog(static_cast<unsigned int>(message.size())) && m_currentFile.is_open())
        {
            m_currentFile<<message;
            m_currentFile.flush();
        }
    }

    std::vector<std::string> LogFileLogger::getCurrentFileList() const
    {
        std::vector<std::string> resultVec;
        for(const auto& entry: std::filesystem::directory_iterator(m_logDirPath))
        {
            if (std::filesystem::is_regular_file(entry.path()) && entry.path().filename().string().starts_with(m_baseFileName))
            {
                resultVec.emplace_back(entry.path().string());
            }
        }
        return resultVec;
    }

    bool LogFileLogger::readyForLog(unsigned int addedSize)
    {
        std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
	    time_t raw_time = std::chrono::system_clock::to_time_t(tp);
	    std::stringstream ss;
	    ss << std::put_time(std::localtime(&raw_time), "%Y-%m-%d");
        std::string nowDate(ss.str());

        if(nowDate != m_currentFileDate)
        {
            if(m_currentFile.is_open())
            {
                m_currentFile.flush();
                m_currentFile.close();
            }
            m_currentFileDate = nowDate;
        }

        //delete old file
        removeOldFiles(getCurrentFileList());
        //roll over too large file
        doRollOver(getCurrentFileList(),addedSize);

        //open file
        m_currentSize += addedSize;
        if(!m_currentFile.is_open())
        {
            m_currentFile.open(getCurrentLoggerFilePath().c_str(), std::ios::app);
            return m_currentFile.is_open();
        }
        return true;
    }

    void LogFileLogger::removeOldFiles(const std::vector<std::string>& allFiles)
    {
        std::for_each(allFiles.begin(),allFiles.end(),[this](const std::string& filePath){
            if(std::smatch sm; std::regex_match(filePath,sm,std::regex(".*?"+m_baseFileName+"-(\\d{4}-\\d{2}-\\d{2})\\.log.*?")))
            {
                std::tm tm;
                std::istringstream ss(sm[1].str()+" 0:0:0");
                ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S"); // or just %T in this case
                time_t file_time = std::mktime(&tm);
                std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
	            time_t raw_time = std::chrono::system_clock::to_time_t(tp);
                if(std::abs(difftime(raw_time,file_time)) > static_cast<double>(m_maxKeepDays)*24*3600)
                {
                    std::filesystem::remove(std::filesystem::path(filePath));
                }
            }
        });
    }

    std::string LogFileLogger::getCurrentLoggerFilePath() const
    {
        return std::filesystem::path(m_logDirPath).append(m_baseFileName).concat("-"+m_currentFileDate+".log").string();
    }

    void LogFileLogger::doRollOver(const std::vector<std::string>& allFiles,unsigned int addedSize)
    {
        if(!m_currentFile.is_open())
        {
            if (std::string filePath = getCurrentLoggerFilePath(); !filePath.empty() && std::filesystem::exists(filePath))
            {
                m_currentSize = std::filesystem::file_size(filePath);
            }
        }
        if(m_currentSize + addedSize > m_maxSingleSize)
        {
            if(m_currentFile.is_open())
            {
                m_currentFile.flush();
                m_currentFile.close();
            }
            std::vector<std::string> renameVec;
            std::for_each(allFiles.begin(),allFiles.end(),[this,&renameVec](const std::string& filePath){
                
                if(std::smatch sm; std::regex_match(filePath,sm,std::regex(".*?"+m_baseFileName+"-"+m_currentFileDate+"\\.log.*?")))
                {
                    renameVec.push_back(filePath);
                }
            });
            if(!renameVec.empty())
            {
                std::stable_sort(renameVec.begin(),renameVec.end(),[](const std::string& ls, const std::string& rs){
                    if(std::smatch sm1,sm2; std::regex_match(ls,sm1,std::regex(".*?\\.log$")))
                    {
                        return false;
                    }
                    else if(std::regex_match(rs,sm2,std::regex(".*?\\.log$")))
                    {
                        return true;
                    }
                    else if(std::regex_match(ls,sm1,std::regex(".*?\\.log\\.(\\d+)$")) && std::regex_match(rs,sm2,std::regex(".*?\\.log\\.(\\d+)$")))
                    {
                        return std::stoi(sm1[1].str()) > std::stoi(sm2[1].str()); 
                    }
                    return true;
                });
                std::for_each(renameVec.begin(),renameVec.end(),[](const std::string& renamePath){
                    if(std::smatch sm; std::regex_match(renamePath,sm,std::regex(".*?\\.log$")))
                    {
                        std::filesystem::rename(renamePath,renamePath+".1");
                    }
                    else if(std::regex_match(renamePath,sm,std::regex(".*?\\.log\\.(\\d+)$")))
                    {
                        std::filesystem::rename(renamePath,std::regex_replace(renamePath,std::regex("(\\d+)$"),std::to_string(std::stoi(sm[1].str())+1)));
                    }
                });
            }
            m_currentSize = 0;
        }
    }
}
