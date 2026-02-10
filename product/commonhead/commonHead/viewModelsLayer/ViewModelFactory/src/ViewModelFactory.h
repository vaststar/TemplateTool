#pragma once

#include <memory>

#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}

namespace commonHead::viewModels{
class ViewModelFactory final: public IViewModelFactory
{
public:
    explicit ViewModelFactory(commonHead::ICommonHeadFrameworkWPtr commonHeadFramework);
    ViewModelFactory(const ViewModelFactory&) = delete;
    ViewModelFactory(ViewModelFactory&&) = delete;
    ViewModelFactory& operator=(const ViewModelFactory&) = delete;
    ViewModelFactory& operator=(ViewModelFactory&&) = delete;
    ~ViewModelFactory();

    [[nodiscard]] std::shared_ptr<commonHead::viewModels::IInvocationViewModel> createInvocationViewModelInstance() const override;
    [[nodiscard]] std::shared_ptr<commonHead::viewModels::IAppUIViewModel> createAppUIViewModelInstance() const override;
    [[nodiscard]] std::shared_ptr<commonHead::viewModels::IClientInfoViewModel> createClientInfoViewModelInstance() const override;
    [[nodiscard]] std::shared_ptr<commonHead::viewModels::IContactListViewModel> createContactListViewModelInstance() const override;
    [[nodiscard]] std::shared_ptr<commonHead::viewModels::IMainWindowViewModel> createMainWindowViewModelInstance() const override;
    [[nodiscard]] std::shared_ptr<commonHead::viewModels::IMediaCameraViewModel> createMediaCameraViewModelInstance() const override;
    [[nodiscard]] std::shared_ptr<commonHead::viewModels::IStabilityViewModel> createStabilityViewModelInstance() const override;

private:
    template <typename T, typename... Args>
    std::shared_ptr<T> createViewModelInstance(const Args&... args) const
    {
        return T::createInstance(args..., mCommonHeadFramework);
    }

private:
    commonHead::ICommonHeadFrameworkWPtr mCommonHeadFramework;
};
}
