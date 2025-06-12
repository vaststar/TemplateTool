#pragma once

#include <memory>
#include <QObject>

#include <UIFabrication/UIFabricationExport.h>

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
}

namespace UIFabrication{
class UIFabrication_EXPORT IViewModelFactory: public QObject
{
Q_OBJECT
public:
    virtual ~IViewModelFactory() = default;
    virtual std::shared_ptr<commonHead::viewModels::IAppUIViewModel> createAppUIViewModelInstance() const = 0;
    virtual std::shared_ptr<commonHead::viewModels::IInvocationViewModel> createInvocationViewModelInstance() const = 0;
    virtual std::shared_ptr<commonHead::viewModels::IClientInfoViewModel> createClientInfoViewModelInstance() const = 0;
    virtual std::shared_ptr<commonHead::viewModels::IContactListViewModel> createContactListViewModelInstance() const = 0;
    virtual std::shared_ptr<commonHead::viewModels::IMainWindowViewModel> createMainWindowViewModelInstance() const = 0;
    virtual std::shared_ptr<commonHead::viewModels::IMediaCameraViewModel> createMediaCameraViewModelInstance() const = 0;
public:
    static std::unique_ptr<IViewModelFactory> createInstance(commonHead::ICommonHeadFrameworkWPtr commonHeadFramework);
};
}