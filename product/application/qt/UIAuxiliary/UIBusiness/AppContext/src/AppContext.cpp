#include <AppContext/AppContext.h>

#include <UIAppCore/UIApplication.h>
#include <UIAppCore/UIQmlEngine.h>
#include <UIFabrication/IUIViewFactory.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <UIManager/IUIManagerProvider.h>


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start Impl Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class AppContext::Impl
{
public:
    Impl(QPointer<UIAppCore::UIApplication> application, QPointer<UIAppCore::UIQmlEngine> qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework);

    const std::unique_ptr<UIFabrication::IUIViewFactory> mViewFactory;
    const std::shared_ptr<commonHead::viewModels::IViewModelFactory> mViewModelFactory;
    const std::unique_ptr<UIManager::IUIManagerProvider> mManagerProvider;
    const QPointer<UIAppCore::UIApplication> mApplication;
    const QPointer<UIAppCore::UIQmlEngine> mQmlEngine;
};

AppContext::Impl::Impl(QPointer<UIAppCore::UIApplication> application, QPointer<UIAppCore::UIQmlEngine> qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
    : mApplication(application)
    , mQmlEngine(qmlEngine)
    , mViewModelFactory(commonHead::viewModels::IViewModelFactory::createInstance(commonheadFramework))
    , mViewFactory(std::move(UIFabrication::IUIViewFactory::createInstance(qmlEngine)))
    , mManagerProvider(std::move(UIManager::IUIManagerProvider::createInstance(application, qmlEngine, commonheadFramework)))
{
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish Impl Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

AppContext::AppContext(QPointer<UIAppCore::UIApplication> application, QPointer<UIAppCore::UIQmlEngine> qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
    : mImpl(std::make_unique<AppContext::Impl>(application, qmlEngine, commonheadFramework))
{

}

AppContext::~AppContext()
{

}

QPointer<UIFabrication::IUIViewFactory> AppContext::getViewFactory() const
{
    return mImpl->mViewFactory.get();
}

std::shared_ptr<commonHead::viewModels::IViewModelFactory> AppContext::getViewModelFactory() const
{
    return mImpl->mViewModelFactory;
}

QPointer<UIAppCore::UIApplication> AppContext::getApplication() const
{
    return mImpl->mApplication;
}

QPointer<UIAppCore::UIQmlEngine> AppContext::getQmlEngine() const
{
    return mImpl->mQmlEngine;
}

QPointer<UIManager::IUIManagerProvider> AppContext::getManagerProvider() const
{
    return mImpl->mManagerProvider.get();
}
