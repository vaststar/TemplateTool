#pragma once

#include <memory>
#include <QObject>

#include <UIFabrication/IViewModelFactory.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}


namespace UIFabrication{
class ViewModelFactory final: public IViewModelFactory
{
Q_OBJECT
public:
    explicit ViewModelFactory(commonHead::ICommonHeadFrameworkWPtr commonHeadFramework);
    ViewModelFactory(const ViewModelFactory&) = delete;
    ViewModelFactory(ViewModelFactory&&) = delete;
    ViewModelFactory& operator=(const ViewModelFactory&) = delete;
    ViewModelFactory& operator=(ViewModelFactory&&) = delete;
    ~ViewModelFactory();

    virtual std::shared_ptr<commonHead::viewModels::IInvocationViewModel> createInvocationViewModelInstance() const override;
    virtual std::shared_ptr<commonHead::viewModels::IAppUIViewModel> createAppUIViewModelInstance() const override;
    virtual std::shared_ptr<commonHead::viewModels::IClientInfoViewModel> createClientInfoViewModelInstance() const override;
    virtual std::shared_ptr<commonHead::viewModels::IContactListViewModel> createContactListViewModelInstance() const override;
    virtual std::shared_ptr<commonHead::viewModels::IMainWindowViewModel> createMainWindowViewModelInstance() const override;
    virtual std::shared_ptr<commonHead::viewModels::IMediaCameraViewModel> createMediaCameraViewModelInstance() const override;
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