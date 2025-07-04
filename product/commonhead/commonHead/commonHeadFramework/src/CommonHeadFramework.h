#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace commonHead{
class CommonHeadFramework: public ICommonHeadFramework, std::enable_shared_from_this<CommonHeadFramework>
{
public:
    CommonHeadFramework(ucf::framework::ICoreFrameworkWPtr coreframework);
    virtual void initCommonheadFramework() override;
    virtual void exitCommonheadFramework() override;
    virtual std::string getName() const override;
public:
    virtual IResourceLoaderPtr getResourceLoader() const override;
    virtual IServiceLocatorPtr getServiceLocator() const override;
private:
    ucf::framework::ICoreFrameworkWPtr getCoreFramework() const;
private:
    ucf::framework::ICoreFrameworkWPtr mCoreframeworkWPtr;
    IServiceLocatorPtr mServiceLocator;
    IResourceLoaderPtr mResourceLoader;
};
}