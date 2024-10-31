#pragma once
#include <memory>
namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}
class CoreViewModelFactory final
{
public:
    explicit CoreViewModelFactory(commonHead::ICommonHeadFrameworkWPtr commonHeadFramework);
    CoreViewModelFactory(const CoreViewModelFactory&) = delete;
    CoreViewModelFactory(CoreViewModelFactory&&) = delete;
    CoreViewModelFactory& operator=(const CoreViewModelFactory&) = delete;
    CoreViewModelFactory& operator=(CoreViewModelFactory&&) = delete;
    ~CoreViewModelFactory();

    template <typename T, typename... Args>
    std::shared_ptr<T> createViewModelInstance(const Args&... args) const
    {
        return T::createInstance(args..., mCommonHeadFramework);
    }
private:
    commonHead::ICommonHeadFrameworkWPtr mCommonHeadFramework;
};