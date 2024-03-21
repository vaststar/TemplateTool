#include "CommonHeadFramework/ICommonHeadFramework.h"

class CommonHeadFramework: public ICommonHeadFramework
{
public:
CommonHeadFramework();
    virtual std::string getName() const override;
};