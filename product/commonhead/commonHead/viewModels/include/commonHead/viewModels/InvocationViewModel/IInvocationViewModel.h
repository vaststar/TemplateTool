#pragma once

#include <vector>
#include <memory>
#include <string>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWptr = std::weak_ptr<ICommonHeadFramework>;
}

namespace commonHead::viewModels{
class COMMONHEAD_EXPORT IInvocationViewModelCallback
{
public:
    virtual ~IInvocationViewModelCallback() = default;
};

class COMMONHEAD_EXPORT IInvocationViewModel: public virtual commonHead::utilities::IVMNotificationHelper<IInvocationViewModelCallback>
{
public:
    virtual std::string getViewModelName() const = 0;

    virtual void processStartupParameters() = 0;
    [[nodiscard]] virtual std::vector<std::string> getStartupParameters() const = 0;
    virtual void processCommandMessage(const std::string& message) = 0;
public:
    static std::shared_ptr<IInvocationViewModel> createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};
}