#include "CoreFrameworkExport.h"
#include "ICoreFramework.h"
class COREFRAMEWORK_EXPORT CoreFramework: public ICoreFramework
{
public:
    virtual std::string getName() override;
};