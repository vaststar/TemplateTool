#pragma once

#include <string>
#include <memory>
#include <UISingleInstance/UISingleInstanceExport.h>

namespace UIUtilities{
class UISingleInstance_EXPORT UISingleInstanceChecker final
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
