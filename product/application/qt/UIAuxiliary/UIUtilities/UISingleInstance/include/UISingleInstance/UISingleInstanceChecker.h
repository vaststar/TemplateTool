#pragma once

#include <string>
#include <memory>
#include <UIUtilities/UIUtilitiesCommonFile/UIUtilitiesExport.h>

namespace UIUtilities{
class UIUtilities_EXPORT UISingleInstanceChecker final
{
public:
    explicit UISingleInstanceChecker(const std::string &name);
    ~UISingleInstanceChecker();
    
    UISingleInstanceChecker(UISingleInstanceChecker&& other) = delete;
    UISingleInstanceChecker& operator=(UISingleInstanceChecker&& other) = delete;

    UISingleInstanceChecker(const UISingleInstanceChecker&) = delete;
    UISingleInstanceChecker& operator=(const UISingleInstanceChecker&) = delete;

    bool tryToRun();

private:
    class Impl;    
    std::unique_ptr<Impl> pImpl;
};
}
