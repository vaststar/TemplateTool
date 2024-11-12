#pragma once
#include <memory>
#include <UIFabrication/UIFabricationExport.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}


namespace UIFabrication{
class UIFabrication_EXPORT ViewModelFactory final
{
public:
    explicit ViewModelFactory(commonHead::ICommonHeadFrameworkWPtr commonHeadFramework);
    ViewModelFactory(const ViewModelFactory&) = delete;
    ViewModelFactory(ViewModelFactory&&) = delete;
    ViewModelFactory& operator=(const ViewModelFactory&) = delete;
    ViewModelFactory& operator=(ViewModelFactory&&) = delete;
    ~ViewModelFactory();

    template <typename T, typename... Args>
    std::shared_ptr<T> createViewModelInstance(const Args&... args) const
    {
        return T::createInstance(args..., mCommonHeadFramework);
    }
private:
    commonHead::ICommonHeadFrameworkWPtr mCommonHeadFramework;
};
}