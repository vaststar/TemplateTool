#pragma once

#include <memory>

#include <ucf/CoreFramework/ICoreFramework.h>
#include <Utilities/NotificationHelper/NotificationHelper.h>

#include "ServiceAccessor.h"


namespace ucf{
class CoreFramework final:  public virtual ICoreFramework,
                            public virtual ServiceAccessor,
                            public virtual Utilities::NotificationHelper<ICoreFrameworkCallback>,
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
}