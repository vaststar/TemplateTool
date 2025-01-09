#include <UIManager/ThemeManager.h>

#include <map>
#include <QLocale>
#include <QTranslator>

#include <UICore/CoreApplication.h>
#include <UICore/CoreQmlEngine.h>

#include "LoggerDefine.h"

namespace UIManager{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start Impl Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class ThemeManager::Impl
{
public:
    Impl(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine);

    const QPointer<UICore::CoreApplication> mApplication;
    const QPointer<UICore::CoreQmlEngine> mQmlEngine;

    QPointer<UIData::UIFontSet> getFontSet() const;
private:
    std::unique_ptr<UIData::UIFontSet> mFontSet;
};

ThemeManager::Impl::Impl(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine)
    : mApplication(application)
    , mQmlEngine(qmlEngine)
{
    mFontSet = std::make_unique<UIData::UIFontSet>();
}

QPointer<UIData::UIFontSet> ThemeManager::Impl::getFontSet() const
{
    return mFontSet.get();
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish Impl Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

ThemeManager::ThemeManager(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine)
    : mImpl(std::make_unique<ThemeManager::Impl>(application, qmlEngine))
{
    UIManager_LOG_DEBUG("");
}

ThemeManager::~ThemeManager()
{

}

void ThemeManager::test()
{
    UIManager_LOG_DEBUG("");
}

UIData::UIColorSet* ThemeManager::getColorSet()
{
    return new UIData::UIColorSet("test", QColor(158,254,125));
}

// QPointer<UIData::UIFontSet> ThemeManager::getFontSet()
// {
//     return mImpl->getFontSet();
// }
QFont ThemeManager::getFont(UIData::UIFontSet::UIFontSize size, UIData::UIFontSet::UIFontWeight weight, bool isItalic)
{
    return mImpl->getFontSet()->getFont(size, weight, isItalic);
}
}
