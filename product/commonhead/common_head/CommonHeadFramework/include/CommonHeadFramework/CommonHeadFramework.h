#include "CommonHeadFramework/ICommonHeadFramework.h"

class ICoreFramework;
using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;

class CommonHeadFramework: public ICommonHeadFramework, std::enable_shared_from_this<CommonHeadFramework>
{
public:
    CommonHeadFramework(ICoreFrameworkWPtr coreframework);
    virtual std::string getName() const override;
    virtual ICoreFrameworkWPtr getCoreFramework() const override;
private:
    ICoreFrameworkWPtr mCoreframeworkWPtr;
};