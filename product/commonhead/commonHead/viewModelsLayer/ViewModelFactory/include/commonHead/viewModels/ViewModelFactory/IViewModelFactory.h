#pragma once

#include <memory>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}

namespace commonHead::viewModels{
    class IAppUIViewModel;
    class IClientInfoViewModel;
    class IContactListViewModel;
    class IMainWindowViewModel;
    class IMediaCameraViewModel;
    class IInvocationViewModel;
    class IStabilityViewModel;
}

namespace commonHead::viewModels{
class COMMONHEAD_EXPORT IViewModelFactory
{
public:
    IViewModelFactory() = default;
    IViewModelFactory(const IViewModelFactory&) = delete;
    IViewModelFactory(IViewModelFactory&&) = delete;
    IViewModelFactory& operator=(const IViewModelFactory&) = delete;
    IViewModelFactory& operator=(IViewModelFactory&&) = delete;
    virtual ~IViewModelFactory() = default;

public:
    [[nodiscard]] virtual std::shared_ptr<commonHead::viewModels::IAppUIViewModel> createAppUIViewModelInstance() const = 0;
    [[nodiscard]] virtual std::shared_ptr<commonHead::viewModels::IInvocationViewModel> createInvocationViewModelInstance() const = 0;
    [[nodiscard]] virtual std::shared_ptr<commonHead::viewModels::IClientInfoViewModel> createClientInfoViewModelInstance() const = 0;
    [[nodiscard]] virtual std::shared_ptr<commonHead::viewModels::IContactListViewModel> createContactListViewModelInstance() const = 0;
    [[nodiscard]] virtual std::shared_ptr<commonHead::viewModels::IMainWindowViewModel> createMainWindowViewModelInstance() const = 0;
    [[nodiscard]] virtual std::shared_ptr<commonHead::viewModels::IMediaCameraViewModel> createMediaCameraViewModelInstance() const = 0;
    [[nodiscard]] virtual std::shared_ptr<commonHead::viewModels::IStabilityViewModel> createStabilityViewModelInstance() const = 0;

public:
    static std::shared_ptr<IViewModelFactory> createInstance(commonHead::ICommonHeadFrameworkWPtr commonHeadFramework);
};
}
