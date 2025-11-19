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

namespace commonHead::viewModels::model{
    enum class LanguageType;
}

namespace commonHead::viewModels{
class COMMONHEAD_EXPORT IAppUIViewModelCallback
{
public:
    IAppUIViewModelCallback() = default;
    IAppUIViewModelCallback(const IAppUIViewModelCallback&) = delete;
    IAppUIViewModelCallback(IAppUIViewModelCallback&&) = delete;
    IAppUIViewModelCallback& operator=(const IAppUIViewModelCallback&) = delete;
    IAppUIViewModelCallback& operator=(IAppUIViewModelCallback&&) = delete;
    virtual ~IAppUIViewModelCallback() = default;
};

class COMMONHEAD_EXPORT IAppUIViewModel: public virtual commonHead::utilities::IVMNotificationHelper<IAppUIViewModelCallback>
{
public:
    IAppUIViewModel() = default;
    IAppUIViewModel(const IAppUIViewModel&) = delete;
    IAppUIViewModel(IAppUIViewModel&&) = delete;
    IAppUIViewModel& operator=(const IAppUIViewModel&) = delete;
    IAppUIViewModel& operator=(IAppUIViewModel&&) = delete;
    virtual ~IAppUIViewModel() = default;
public:
    virtual std::string getViewModelName() const = 0;
    /**
     * @brief Initializes the application.
     * This method should be called to set up the application environment.
     * It may include loading resources, setting up configurations, etc.
     */
    virtual void initApplication() = 0;
public:
    static std::shared_ptr<IAppUIViewModel> createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};
}