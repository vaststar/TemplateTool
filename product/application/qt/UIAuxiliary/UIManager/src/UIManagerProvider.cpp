#include <UIManager/UIManagerProvider.h>

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>

#include <UICore/CoreApplication.h>
#include <UICore/CoreQmlEngine.h>
#include <UIManager/TranslatorManager.h>
#include <UIManager/ThemeManager.h>

namespace UIManager{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start Impl Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class UIManagerProvider::Impl
{
public:
    explicit Impl(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework);

    std::unique_ptr<TranslatorManager> mTranslatorManager;
    std::unique_ptr<ThemeManager> mThemeManager;
};

UIManagerProvider::Impl::Impl(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
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

UIManagerProvider::UIManagerProvider(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
    : mImpl(std::make_unique<UIManagerProvider::Impl>(application, qmlEngine, commonheadFramework))
{
}

UIManagerProvider::~UIManagerProvider()
{

}

QPointer<TranslatorManager> UIManagerProvider::getTranslatorManager() const
{
    return mImpl->mTranslatorManager.get();
}

QPointer<ThemeManager> UIManagerProvider::getThemeManager() const
{
    return mImpl->mThemeManager.get();
}
}
