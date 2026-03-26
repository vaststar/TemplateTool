#pragma once

#include <memory>

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>
#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

#include "ServiceAccessor.h"

namespace ucf::framework{
class SERVICE_EXPORT CoreFramework final:  public virtual ICoreFramework,
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
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}