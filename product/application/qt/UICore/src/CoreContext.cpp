#include <UICore/CoreContext.h>

#include <UICore/CoreViewFactory.h>
CoreContext::CoreContext(std::unique_ptr<CoreViewFactory>&& viewFactory)
    : mViewFactory(std::move(viewFactory))
{

}

CoreContext::~CoreContext()
{

}

const std::unique_ptr<CoreViewFactory>& CoreContext::getViewFactory() const
{
    return mViewFactory;
}