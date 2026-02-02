#pragma once

#include <thread>
#include <memory>
#include <string>
#include <atomic>

#include <commonHead/commonHeadUtils/VMNotificationHelper/VMNotificationHelper.h>
#include <ucf/Services/ImageService/ImageTypes.h>
#include <commonHead/viewModels/InvocationViewModel/IInvocationViewModel.h>

namespace commonHead::viewModels{
class InvocationViewModel: public virtual IInvocationViewModel, 
                            public virtual commonHead::utilities::VMNotificationHelper<IInvocationViewModelCallback>,
                            public std::enable_shared_from_this<InvocationViewModel>
{
public:
    explicit InvocationViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    InvocationViewModel() = default;
    InvocationViewModel(const InvocationViewModel&) = delete;
    InvocationViewModel(InvocationViewModel&&) = delete;
    InvocationViewModel& operator=(const InvocationViewModel&) = delete;
    InvocationViewModel& operator=(InvocationViewModel&&) = delete;
    ~InvocationViewModel();
public:
    virtual std::string getViewModelName() const override;

    virtual void processStartupParameters() override;
    virtual std::vector<std::string> getStartupParameters() const override;
    virtual void processCommandMessage(const std::string& message) override;
protected:
    virtual void init() override;
};
}