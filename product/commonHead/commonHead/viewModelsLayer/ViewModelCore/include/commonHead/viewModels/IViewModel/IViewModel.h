#pragma once

#include <memory>
#include <string>

#include <commonHead/viewModels/IViewModel/ViewModelCoreExport.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWptr = std::weak_ptr<ICommonHeadFramework>;
}

namespace commonHead::viewModels{
class VIEW_MODEL_CORE_API IViewModelCallback
{
public:
    IViewModelCallback() = default;
    IViewModelCallback(const IViewModelCallback&) = delete;
    IViewModelCallback(IViewModelCallback&&) = delete;
    IViewModelCallback& operator=(const IViewModelCallback&) = delete;
    IViewModelCallback& operator=(IViewModelCallback&&) = delete;
    virtual ~IViewModelCallback() = default;
};

class VIEW_MODEL_CORE_API IViewModel
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
