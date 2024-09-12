#include <UICore/CoreContext.h>

#include <UICore/CoreViewFactory.h>
#include <UICore/CoreViewModelFactory.h>
CoreContext::CoreContext(std::unique_ptr<CoreViewModelFactory>&& viewModelFactory, std::unique_ptr<CoreViewFactory>&& viewFactory)
    : mViewModelFactory(std::move(viewModelFactory))
    , mViewFactory(std::move(viewFactory))
{

}

CoreContext::~CoreContext()
{

}

const std::unique_ptr<CoreViewFactory>& CoreContext::getViewFactory() const
{
    return mViewFactory;
}


const std::unique_ptr<CoreViewModelFactory>& CoreContext::getViewModelFactory() const
{
    return mViewModelFactory;
}