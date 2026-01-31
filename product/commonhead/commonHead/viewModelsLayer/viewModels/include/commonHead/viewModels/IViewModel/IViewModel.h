#pragma once

#include <string>
#include <vector>
#include <memory>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWptr = std::weak_ptr<ICommonHeadFramework>;
}

namespace commonHead::viewModels{
class COMMONHEAD_EXPORT IViewModelCallback
{
public:
    IViewModelCallback() = default;
    IViewModelCallback(const IViewModelCallback&) = delete;
    IViewModelCallback(IViewModelCallback&&) = delete;
    IViewModelCallback& operator=(const IViewModelCallback&) = delete;
    IViewModelCallback& operator=(IViewModelCallback&&) = delete;
    virtual ~IViewModelCallback() = default;
};

class COMMONHEAD_EXPORT IViewModel
{
public:
    explicit IViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    IViewModel(const IViewModel&) = delete;
    IViewModel(IViewModel&&) = delete;
    IViewModel& operator=(const IViewModel&) = delete;
    IViewModel& operator=(IViewModel&&) = delete;
    virtual ~IViewModel() = default;
public:
    virtual std::string getViewModelName() const = 0;
    void initViewModel();
protected:
    virtual void init() = 0;
    commonHead::ICommonHeadFrameworkWptr getCommonHeadFramework() const;
private:
    commonHead::ICommonHeadFrameworkWptr mCommonHeadFrameworkWptr;
};
}