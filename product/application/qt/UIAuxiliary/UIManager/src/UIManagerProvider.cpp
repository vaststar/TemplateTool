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

UIManagerProvider::UIManagerProvider(QPointer<UICore::CoreApplication> application, QPointer<UICore::CoreQmlEngine> qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
    : mTranslatorManager(std::make_unique<TranslatorManager>(application, qmlEngine))
    , mThemeManager(std::make_unique<ThemeManager>(application, qmlEngine, commonheadFramework))
{
    qmlEngine->rootContext()->setContextProperty("ThemeManager",mThemeManager.get());
}

UIManagerProvider::~UIManagerProvider()
{

}

QPointer<ITranslatorManager> UIManagerProvider::getTranslatorManager() const
{
    return mTranslatorManager.get();
}

QPointer<IThemeManager> UIManagerProvider::getThemeManager() const
{
    return mThemeManager.get();
}
}
