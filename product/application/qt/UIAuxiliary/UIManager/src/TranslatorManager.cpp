#include "TranslatorManager.h"

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
class TranslatorManager::Impl
{
public:
    Impl(QPointer<UICore::CoreApplication> application, QPointer<UICore::CoreQmlEngine> qmlEngine);

    const QPointer<UICore::CoreApplication> mApplication;
    const QPointer<UICore::CoreQmlEngine> mQmlEngine;
    const std::unique_ptr<QTranslator> mTranslator;

    QString mCurrentLanguage;

    std::string getLanguageString(UILanguage::LanguageType languageType) const;
    std::map<UILanguage::LanguageType, std::string> getLanguageMap() const;
};

TranslatorManager::Impl::Impl(QPointer<UICore::CoreApplication> application, QPointer<UICore::CoreQmlEngine> qmlEngine)
    : mApplication(application)
    , mQmlEngine(qmlEngine)
    , mTranslator(std::make_unique<QTranslator>())
{
}

std::map<UILanguage::LanguageType, std::string> TranslatorManager::Impl::getLanguageMap() const
{
    const std::map<UILanguage::LanguageType, std::string> languageMap = {
        {UILanguage::LanguageType::LanguageType_ENGLISH, "en"},
        {UILanguage::LanguageType::LanguageType_CHINESE_SIMPLIFIED, "zh-CN"},
        {UILanguage::LanguageType::LanguageType_CHINESE_TRADITIONAL, "zh-TW"},
        {UILanguage::LanguageType::LanguageType_FRENCH, "fr"},
        {UILanguage::LanguageType::LanguageType_GERMAN, "de"},
        {UILanguage::LanguageType::LanguageType_ITALIAN, "it"},
        {UILanguage::LanguageType::LanguageType_SPANISH, "es"},
        {UILanguage::LanguageType::LanguageType_PORTUGUESE, "pt"},
        {UILanguage::LanguageType::LanguageType_JAPANESE, "ja"},
        {UILanguage::LanguageType::LanguageType_KOREAN, "ko"},
        {UILanguage::LanguageType::LanguageType_RUSSIAN, "ru"}
    };
    return languageMap;
}

std::string TranslatorManager::Impl::getLanguageString(UILanguage::LanguageType languageType) const
{
    const auto& languageMap = getLanguageMap();
    if (auto iter = languageMap.find(languageType); iter != languageMap.end())
    {
        return iter->second;
    }
    else
    {
        return "en";
    }
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish Impl Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

TranslatorManager::TranslatorManager(QPointer<UICore::CoreApplication> application, QPointer<UICore::CoreQmlEngine> qmlEngine)
    : mImpl(std::make_unique<TranslatorManager::Impl>(application, qmlEngine))
{
    UIManager_LOG_DEBUG("");
}

TranslatorManager::~TranslatorManager()
{

}

void TranslatorManager::loadTranslation(const QString& language)
{
    if (language.isEmpty())
    {
        UIManager_LOG_WARN("empty language, check your code.");
        return;
    }

    if (!mImpl->mApplication)
    {
        UIManager_LOG_WARN("application not found");
        return;
    }

    if (language == mImpl->mCurrentLanguage)
    {
        UIManager_LOG_WARN("won't load same translation file, language:" << language.toStdString());
        return;
    }

    if (!mImpl->mApplication->removeTranslator(mImpl->mTranslator.get()))
    {//if no translator installed, removeTranslator will return false and this is expected
        UIManager_LOG_INFO("remove translation file failed");
    }

    if (QString translationFileName = QString("app_translations_%1").arg(language); !mImpl->mTranslator->load(translationFileName, ":/i18n"))
    {
        UIManager_LOG_WARN("load translation file failed, language:" << language.toStdString() << ", file:" << translationFileName.toStdString());
        return;
    }

    if (!mImpl->mApplication->installTranslator(mImpl->mTranslator.get()))
    {
        UIManager_LOG_WARN("install translation file failed, language:" << language.toStdString());
        return;
    }

    mImpl->mCurrentLanguage = language;
    UIManager_LOG_INFO("load translation file succeed, language:" << language.toStdString());

    if (mImpl->mQmlEngine)
    {
        mImpl->mQmlEngine->retranslate();
    }
    emit languageChanged(language);
}

void TranslatorManager::loadTranslation(UILanguage::LanguageType languageType)
{
    loadTranslation(QString::fromStdString(mImpl->getLanguageString(languageType)));
}

void TranslatorManager::loadSystemTranslation()
{
    QString systemLanguage = QLocale::system().name().section('_', 0, 0);
    loadTranslation(systemLanguage);
}
}
