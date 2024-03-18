#include "ServiceExport.h"
#include "ICoreFramework.h"
class SERVICE_EXPORT CoreFramework final: public ICoreFramework
{
public:
    CoreFramework();
public:
    virtual std::string getName() const override;
    virtual std::vector<std::weak_ptr<IService>> getServices() const override;
};