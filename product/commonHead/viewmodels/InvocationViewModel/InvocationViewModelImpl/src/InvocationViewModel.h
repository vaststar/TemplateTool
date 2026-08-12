#pragma once

#include <memory>
#include <string>
#include <vector>

#include <commonHead/commonHeadUtils/VMNotificationHelper/VMNotificationHelper.h>
#include <commonHead/viewModels/InvocationViewModel/IInvocationViewModel.h>

namespace commonHead::viewModels{

class InvocationViewModel: public virtual IInvocationViewModel,
                           public virtual commonHead::utilities::VMNotificationHelper<IInvocationViewModelCallback>,
                           public std::enable_shared_from_this<InvocationViewModel>
{
public:
    explicit InvocationViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    InvocationViewModel(const InvocationViewModel&) = delete;
    InvocationViewModel(InvocationViewModel&&) = delete;
    InvocationViewModel& operator=(const InvocationViewModel&) = delete;
    InvocationViewModel& operator=(InvocationViewModel&&) = delete;
    ~InvocationViewModel() override;
public:
    std::string getViewModelName() const override;
    void processStartupParameters() override;
    std::vector<std::string> getStartupParameters() const override;
    void processCommandMessage(const std::string& message) override;
protected:
    void init() override;
};

}
