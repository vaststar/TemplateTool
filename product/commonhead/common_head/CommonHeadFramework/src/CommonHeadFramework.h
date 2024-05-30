#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>

namespace ucf {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace commonHead{
class CommonHeadFramework: public ICommonHeadFramework, std::enable_shared_from_this<CommonHeadFramework>
{
public:
    CommonHeadFramework(ucf::ICoreFrameworkWPtr coreframework);
    virtual std::string getName() const override;
    virtual ucf::ICoreFrameworkWPtr getCoreFramework() const override;
private:
    ucf::ICoreFrameworkWPtr mCoreframeworkWPtr;
};
}