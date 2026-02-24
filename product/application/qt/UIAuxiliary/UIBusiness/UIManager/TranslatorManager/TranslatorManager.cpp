#include "TranslatorManager.h"

#include <map>
#include <QLocale>

#include "LoggerDefine/LoggerDefine.h"

namespace UIManager{
TranslatorManager::TranslatorManager(QPointer<UIAppCore::UIApplication> application, QPointer<UIAppCore::UIQmlEngine> qmlEngine)
    : mApplication(application)
    , mQmlEngine(qmlEngine)
    , mTranslator(std::make_unique<QTranslator>())
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

    if (!mApplication)
    {
        UIManager_LOG_WARN("application not found");
        return;
    }

    if (language == mCurrentLanguage)
    {
        UIManager_LOG_WARN("won't load same translation file, language:" << language.toStdString());
        return;
    }

    if (!mApplication->removeTranslator(mTranslator.get()))
    {//if no translator installed, removeTranslator will return false and this is expected
        UIManager_LOG_INFO("remove translation file failed");
    }

    //ps: if the language is en, it may failed because the translation.ts is empty
    if (QString translationFileName = QString("app_translations_%1").arg(language); !mTranslator->load(translationFileName, ":/i18n"))
    {
        UIManager_LOG_WARN("load translation file failed, language:" << language.toStdString() << ", file:" << translationFileName.toStdString());
        return;
    }

    if (!mApplication->installTranslator(mTranslator.get()))
    {
        UIManager_LOG_WARN("install translation file failed, language:" << language.toStdString());
        return;
    }

    mCurrentLanguage = language;
    UIManager_LOG_INFO("load translation file succeed, language:" << language.toStdString());

    if (mQmlEngine)
    {
        mQmlEngine->retranslate();
    }
    emit languageChanged(language);
}

void TranslatorManager::loadTranslation(UILanguage::LanguageType languageType)
{
    loadTranslation(QString::fromStdString(getLanguageString(languageType)));
}

void TranslatorManager::loadSystemTranslation()
{
    QString systemLanguage = QLocale::system().name().section('_', 0, 0);
    loadTranslation(systemLanguage);
}

std::map<UILanguage::LanguageType, std::string> TranslatorManager::getLanguageMap() const
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

std::string TranslatorManager::getLanguageString(UILanguage::LanguageType languageType) const
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
}
