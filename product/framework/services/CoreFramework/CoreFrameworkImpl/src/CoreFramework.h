#pragma once

#include <memory>
#include <vector>

#include <ucf/CoreFramework/CoreFrameworkImplExport.h>
#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

#include "ServiceAccessor.h"

namespace ucf::framework{
class CORE_FRAMEWORK_IMPL_API CoreFramework final:  public virtual ICoreFramework,
                            public virtual ServiceAccessor,
                            public virtual ucf::utilities::NotificationHelper<ICoreFrameworkCallback>,
                            public std::enable_shared_from_this<CoreFramework>
{
public:
    CoreFramework();
    ~CoreFramework();
    CoreFramework(const CoreFramework&) = delete;
    CoreFramework(CoreFramework&&) = delete;
    CoreFramework& operator=(const CoreFramework&) = delete;
    CoreFramework& operator=(CoreFramework&&) = delete;
public:
    //ICoreFramework
    virtual std::string getName() const override;
    virtual void initCoreFramework() override;
    virtual void initServices() override;
    virtual void exitCoreFramework() override;
    virtual void setStartupParameters(const std::vector<std::string>& args) override;
    virtual std::vector<std::string> getStartupParameters() const override;
private:
    // Tears down services in the exact reverse of initialization order.
    void deinitServices();
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}
