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
    QPointer<UIData::UIColorSet> getColorSet() const;
private:
    void initFontSet();
    void initColorSet();
private:
    std::unique_ptr<UIData::UIFontSet> mFontSet;
    std::unique_ptr<UIData::UIColorSet> mColorSet;
};

ThemeManager::Impl::Impl(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine)
    : mApplication(application)
    , mQmlEngine(qmlEngine)
    , mFontSet(std::make_unique<UIData::UIFontSet>())
    , mColorSet(std::make_unique<UIData::UIColorSet>())
{
    initFontSet();
    initColorSet();
}

QPointer<UIData::UIFontSet> ThemeManager::Impl::getFontSet() const
{
    return mFontSet.get();
}

QPointer<UIData::UIColorSet> ThemeManager::Impl::getColorSet() const
{
    return mColorSet.get();
}

void ThemeManager::Impl::initFontSet()
{
    std::vector<std::shared_ptr<UIData::UIFont>> uiFonts;
    uiFonts.emplace_back(std::make_shared<UIData::UIFont>(UIData::UIFont::UIFontFamily::SegoeUI));
    uiFonts.emplace_back(std::make_shared<UIData::UIFont>(UIData::UIFont::UIFontFamily::Consolas));
    uiFonts.emplace_back(std::make_shared<UIData::UIFont>(UIData::UIFont::UIFontFamily::SegoeUIEmoji));
    mFontSet->initFonts(uiFonts);
}

void ThemeManager::Impl::initColorSet()
{

}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish Impl Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

ThemeManager::ThemeManager(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine)
    : QObject(nullptr)
    , mImpl(std::make_unique<ThemeManager::Impl>(application, qmlEngine))
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
QFont ThemeManager::getFont(UIData::UIFont::UIFontSize size, UIData::UIFont::UIFontWeight weight, bool isItalic, UIData::UIFont::UIFontFamily family)
{
    return mImpl->getFontSet()->getFont(family, size, weight, isItalic);
    QFont("Segoe UI", 14, QFont::Normal, false)
}
}
