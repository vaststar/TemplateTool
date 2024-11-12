#include <AppContext/AppContext.h>

#include <UICore/CoreApplication.h>
#include <UICore/CoreQmlEngine.h>
#include <UIFabrication/UIViewFactory.h>
#include <UIFabrication/ViewModelFactory.h>
#include <UIManager/UIManagerProvider.h>


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start Impl Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class AppContext::Impl
{
public:
    Impl(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework);

    const std::unique_ptr<UIFabrication::UIViewFactory> mViewFactory;
    const std::unique_ptr<UIFabrication::ViewModelFactory> mViewModelFactory;
    const std::unique_ptr<UIManager::UIManagerProvider> mManagerProvider;
    const QPointer<UICore::CoreApplication> mApplication;
    const QPointer<UICore::CoreQmlEngine> mQmlEngine;
};

AppContext::Impl::Impl(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
    : mApplication(application)
    , mQmlEngine(qmlEngine)
    , mViewModelFactory(std::make_unique<UIFabrication::ViewModelFactory>(commonheadFramework))
    , mViewFactory(std::make_unique<UIFabrication::UIViewFactory>(qmlEngine))
    , mManagerProvider(std::make_unique<UIManager::UIManagerProvider>(application, qmlEngine))
{
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish Impl Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

AppContext::AppContext(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
    : mImpl(std::make_unique<AppContext::Impl>(application, qmlEngine, commonheadFramework))
{

}

AppContext::~AppContext()
{

}

const std::unique_ptr<UIFabrication::UIViewFactory>& AppContext::getViewFactory() const
{
    return mImpl->mViewFactory;
}


const std::unique_ptr<UIFabrication::ViewModelFactory>& AppContext::getViewModelFactory() const
{
    return mImpl->mViewModelFactory;
}

QPointer<UICore::CoreApplication> AppContext::getApplication() const
{
    return mImpl->mApplication;
}

QPointer<UICore::CoreQmlEngine> AppContext::getQmlEngine() const
{
    return mImpl->mQmlEngine;
}


const std::unique_ptr<UIManager::UIManagerProvider>& AppContext::getManagerProvider() const
{
    return mImpl->mManagerProvider;
}
