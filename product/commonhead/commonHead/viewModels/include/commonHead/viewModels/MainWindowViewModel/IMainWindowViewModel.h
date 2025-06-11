#pragma once

#include <vector>
#include <memory>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWptr = std::weak_ptr<ICommonHeadFramework>;
}

namespace commonHead::viewModels{
class COMMONHEAD_EXPORT IMainWindowViewModelCallback
{
public:
    virtual ~IMainWindowViewModelCallback() = default;
};

class COMMONHEAD_EXPORT IMainWindowViewModel: public virtual commonHead::utilities::IVMNotificationHelper<IMainWindowViewModelCallback>
{
public:
    virtual std::string getViewModelName() const = 0;
public:
    static std::shared_ptr<IMainWindowViewModel> createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};
}