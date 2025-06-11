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
    virtual ~IAppUIViewModelCallback() = default;
};

class COMMONHEAD_EXPORT IAppUIViewModel: public virtual commonHead::utilities::IVMNotificationHelper<IAppUIViewModelCallback>
{
public:
    virtual std::string getViewModelName() const = 0;
    virtual void initDatabase() = 0;
public:
    static std::shared_ptr<IAppUIViewModel> createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};
}