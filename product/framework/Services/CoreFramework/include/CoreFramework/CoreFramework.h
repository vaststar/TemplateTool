// #include "ServiceCommonFile/ServiceExport.h"
#include "CoreFramework/ICoreFramework.h"

#include <memory>
#include "NotificationHelper/NotificationHelperImpl.h"

class CoreFramework final:  public ICoreFramework,
                            // public NotificationHelperImpl<ICoreFrameworkCallback>,
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