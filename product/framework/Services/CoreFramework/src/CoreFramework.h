// #include "ServiceCommonFile/ServiceExport.h"
#include "CoreFramework/ICoreFramework.h"

#include <memory>
#include <Utilities/NotificationHelper/NotificationHelper.h>

class CoreFramework final:  public virtual ICoreFramework,
                            public virtual NotificationHelper<ICoreFrameworkCallback>,
                            public std::enable_shared_from_this<CoreFramework>
{
public:
    CoreFramework();
public:
    virtual std::string getName() const override;
    virtual void initServices() override;

    
    // virtual void registerCallback(CallbackPtr callback) override;
private:
    class CoreFrameworkImpl;
    std::shared_ptr<CoreFrameworkImpl> mImpl;
};