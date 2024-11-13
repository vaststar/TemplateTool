#include <UIManager/TranslatorManager.h>

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
    Impl(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine);

    const QPointer<UICore::CoreApplication> mApplication;
    const QPointer<UICore::CoreQmlEngine> mQmlEngine;
    const std::unique_ptr<QTranslator> mTranslator;

    QString mCurrentLanguage;

    std::string getLanguageString(LanguageType languageType) const;
    std::map<LanguageType, std::string> getLanguageMap() const;
};

TranslatorManager::Impl::Impl(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine)
    : mApplication(application)
    , mQmlEngine(qmlEngine)
    , mTranslator(std::make_unique<QTranslator>())
{
}

std::map<LanguageType, std::string> TranslatorManager::Impl::getLanguageMap() const
{
    const std::map<LanguageType, std::string> languageMap = {
        {LanguageType::ENGLISH, "en"},
        {LanguageType::CHINESE_SIMPLIFIED, "zh-CN"},
        {LanguageType::CHINESE_TRADITIONAL, "zh-TW"},
        {LanguageType::FRENCH, "fr"},
        {LanguageType::GERMAN, "de"},
        {LanguageType::ITALIAN, "it"},
        {LanguageType::SPANISH, "es"},
        {LanguageType::PORTUGUESE, "pt"},
        {LanguageType::JAPANESE, "ja"},
        {LanguageType::KOREAN, "ko"},
        {LanguageType::RUSSIAN, "ru"}
    };
    return languageMap;
}

std::string TranslatorManager::Impl::getLanguageString(LanguageType languageType) const
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

TranslatorManager::TranslatorManager(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine)
    : mImpl(std::make_unique<TranslatorManager::Impl>(application, qmlEngine))
{

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

    if (language == mImpl->mCurrentLanguage)
    {
        UIManager_LOG_WARN("won't load same translation file, language:" << language.toStdString());
        return;
    }

    QString translationFileName = QString("UIVIEW_translations_%1").arg(language);
    if (mImpl->mTranslator->load(translationFileName, ":/i18n"))
    {
        UIManager_LOG_INFO("load translation file succeed, language:" << language.toStdString());
        if (mImpl->mApplication)
        {
            mImpl->mCurrentLanguage = language;
            mImpl->mApplication->removeTranslator(mImpl->mTranslator.get());
            mImpl->mApplication->installTranslator(mImpl->mTranslator.get());
            if (mImpl->mQmlEngine)
            {
                mImpl->mQmlEngine->retranslate();
            }
            emit languageChanged(language);
        }
    }
    else
    {
        UIManager_LOG_WARN("load translation file failed, language:" << language.toStdString());
    }

}

void TranslatorManager::loadTranslation(LanguageType languageType)
{
    loadTranslation(QString::fromStdString(mImpl->getLanguageString(languageType)));
}

void TranslatorManager::loadSystemTranslation()
{
    QString systemLanguage = QLocale::system().name().section('_', 0, 0);
    loadTranslation(systemLanguage);
}
}
