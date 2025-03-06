#include "UIManagerProvider.h"

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>

#include <UICore/CoreApplication.h>
#include <UICore/CoreQmlEngine.h>

#include "TranslatorManager.h"
#include "ThemeManager.h"

namespace UIManager{
std::unique_ptr<IUIManagerProvider> IUIManagerProvider::createInstance(QPointer<UICore::CoreApplication> application, QPointer<UICore::CoreQmlEngine> qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
{
    return std::make_unique<UIManagerProvider>(application, qmlEngine, commonheadFramework);
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start Impl Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class UIManagerProvider::Impl
{
public:
    explicit Impl(QPointer<UICore::CoreApplication> application, QPointer<UICore::CoreQmlEngine> qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework);

    std::unique_ptr<ITranslatorManager> mTranslatorManager;
    std::unique_ptr<IThemeManager> mThemeManager;
};

UIManagerProvider::Impl::Impl(QPointer<UICore::CoreApplication> application, QPointer<UICore::CoreQmlEngine> qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
    : mTranslatorManager(std::make_unique<TranslatorManager>(application, qmlEngine))
    , mThemeManager(std::make_unique<ThemeManager>(application, qmlEngine, commonheadFramework))
{
    qmlEngine->rootContext()->setContextProperty("ThemeManager",mThemeManager.get());
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish Impl Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

UIManagerProvider::UIManagerProvider(QPointer<UICore::CoreApplication> application, QPointer<UICore::CoreQmlEngine> qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
    : mImpl(std::make_unique<UIManagerProvider::Impl>(application, qmlEngine, commonheadFramework))
{
}

UIManagerProvider::~UIManagerProvider()
{

}

QPointer<ITranslatorManager> UIManagerProvider::getTranslatorManager() const
{
    return mImpl->mTranslatorManager.get();
}

QPointer<IThemeManager> UIManagerProvider::getThemeManager() const
{
    return mImpl->mThemeManager.get();
}
}
