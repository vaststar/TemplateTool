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
    class ISettingsViewModel;
    class ISideBarViewModel;
    class IStabilityViewModel;
    class IToolsViewModel;
    class INetworkProxyViewModel;
    class IScreenshotViewModel;
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
    [[nodiscard]] virtual std::shared_ptr<commonHead::viewModels::ISettingsViewModel> createSettingsViewModelInstance() const = 0;
    [[nodiscard]] virtual std::shared_ptr<commonHead::viewModels::ISideBarViewModel> createSideBarViewModelInstance() const = 0;
    [[nodiscard]] virtual std::shared_ptr<commonHead::viewModels::IStabilityViewModel> createStabilityViewModelInstance() const = 0;
    [[nodiscard]] virtual std::shared_ptr<commonHead::viewModels::IToolsViewModel> createToolsViewModelInstance() const = 0;
    [[nodiscard]] virtual std::shared_ptr<commonHead::viewModels::INetworkProxyViewModel> createNetworkProxyViewModelInstance() const = 0;
    [[nodiscard]] virtual std::shared_ptr<commonHead::viewModels::IScreenshotViewModel> createScreenshotViewModelInstance() const = 0;

public:
    static std::shared_ptr<IViewModelFactory> createInstance(commonHead::ICommonHeadFrameworkWPtr commonHeadFramework);
};
}
