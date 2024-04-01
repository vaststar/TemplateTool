#include "ServiceCommonFile/ServiceExport.h"
#include "CoreFramework/ICoreFramework.h"

#include <memory>
class SERVICE_EXPORT CoreFramework final: public ICoreFramework,
                                          public std::enable_shared_from_this<CoreFramework>
{
public:
    CoreFramework();
public:
    virtual std::string getName() const override;
    virtual void initServices() override;
};