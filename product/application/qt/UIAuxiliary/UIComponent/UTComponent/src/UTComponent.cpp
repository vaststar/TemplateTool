#include <UTComponent/UTComponent.h>

#include <UIComponentBase/UIComponentBase.h>
#include "LoggerDefine.h"

void UTComponent::registerUTComponent()
{
    UICOM_LOG_DEBUG("register UTComponent");
    UIComponentBase::registerUIComponentBase();
}