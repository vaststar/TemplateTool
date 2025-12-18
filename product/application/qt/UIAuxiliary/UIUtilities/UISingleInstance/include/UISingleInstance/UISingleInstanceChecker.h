#pragma once

#include <string>
#include <memory>
#include <UIUtilities/UIUtilitiesCommonFile/UIUtilitiesExport.h>

namespace UIUtilities{
class UIUtilities_EXPORT SingleInstanceChecker final
{
public:
    explicit SingleInstanceChecker(const std::string &name);
    ~SingleInstanceChecker();
    
    SingleInstanceChecker(SingleInstanceChecker&& other) = delete;
    SingleInstanceChecker& operator=(SingleInstanceChecker&& other) = delete;

    SingleInstanceChecker(const SingleInstanceChecker&) = delete;
    SingleInstanceChecker& operator=(const SingleInstanceChecker&) = delete;

    bool tryToRun();

private:
    class Impl;    
    std::unique_ptr<Impl> pImpl;
};
}
