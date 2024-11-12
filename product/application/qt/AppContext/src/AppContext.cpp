#include <AppContext/AppContext.h>

#include <UICore/CoreApplication.h>
#include <UICore/CoreQmlEngine.h>
#include <UIFabrication/UIViewFactory.h>
#include <UIFabrication/ViewModelFactory.h>


AppContext::AppContext(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
    : mApplication(application)
    , mQmlEngine(qmlEngine)
    , mViewModelFactory(std::make_unique<UIFabrication::ViewModelFactory>(commonheadFramework))
    , mViewFactory(std::make_unique<UIFabrication::UIViewFactory>(qmlEngine))
{

}

AppContext::~AppContext()
{

}

const std::unique_ptr<UIFabrication::UIViewFactory>& AppContext::getViewFactory() const
{
    return mViewFactory;
}


const std::unique_ptr<UIFabrication::ViewModelFactory>& AppContext::getViewModelFactory() const
{
    return mViewModelFactory;
}

QPointer<UICore::CoreApplication> AppContext::getApplication() const
{
    return mApplication;
}

QPointer<UICore::CoreQmlEngine> AppContext::getQmlEngine() const
{
    return mQmlEngine;
}

