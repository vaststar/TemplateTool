#include <UIFabrication/ViewModelFactory.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>


namespace UIFabrication{
ViewModelFactory::ViewModelFactory(commonHead::ICommonHeadFrameworkWPtr commonHeadFramework)
    : mCommonHeadFramework(commonHeadFramework)
{

}

ViewModelFactory::~ViewModelFactory()
{
    
}
}