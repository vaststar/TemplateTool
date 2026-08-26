#include "TranslatorManager.h"

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <string_view>

#include <QLocale>

#include <UIResourceTranslation/GeneratedTranslationCatalog.h>

#include "LoggerDefine.h"

namespace UIManager {

namespace {

using LanguageType = UILanguage::LanguageType;

struct LanguageEntry
{
    LanguageType type;
    std::string_view locale;
};

constexpr std::array<LanguageEntry, 11> kLanguageEntries{{
    {LanguageType::LanguageType_ENGLISH, "en"},
    {LanguageType::LanguageType_CHINESE_SIMPLIFIED, "zh-Hans"},
    {LanguageType::LanguageType_CHINESE_TRADITIONAL, "zh-Hant"},
    {LanguageType::LanguageType_FRENCH, "fr"},
    {LanguageType::LanguageType_GERMAN, "de"},
    {LanguageType::LanguageType_ITALIAN, "it"},
    {LanguageType::LanguageType_SPANISH, "es"},
    {LanguageType::LanguageType_PORTUGUESE, "pt"},
    {LanguageType::LanguageType_JAPANESE, "ja"},
    {LanguageType::LanguageType_KOREAN, "ko"},
    {LanguageType::LanguageType_RUSSIAN, "ru"}
}};

std::optional<std::string_view> getLanguageTag(LanguageType type)
{
    const auto iter = std::find_if(
        kLanguageEntries.begin(),
        kLanguageEntries.end(),
        [type](const LanguageEntry& entry) {
            return entry.type == type;
        });

    if (iter == kLanguageEntries.end())
    {
        return std::nullopt;
    }
    return iter->locale;
}

std::optional<LanguageType> getLanguageType(std::string_view locale)
{
    const auto iter = std::find_if(
        kLanguageEntries.begin(),
        kLanguageEntries.end(),
        [locale](const LanguageEntry& entry) {
            return entry.locale == locale;
        });

    if (iter == kLanguageEntries.end())
    {
        return std::nullopt;
    }
    return iter->type;
}

bool isPackagedLanguage(std::string_view locale)
{
    return std::find(
               UIResourceTranslation::detail::kPackagedTranslationLocales.begin(),
               UIResourceTranslation::detail::kPackagedTranslationLocales.end(),
               locale) != UIResourceTranslation::detail::kPackagedTranslationLocales.end();
}

QString toQString(std::string_view value)
{
    return QString::fromLatin1(value.data(), static_cast<int>(value.size()));
}

QString getTranslationResource(const QString& language)
{
    return QStringLiteral(":/i18n/app_translations_%1.qm").arg(language);
}

LanguageType getSystemLanguageType(const QLocale& locale)
{
    switch (locale.language())
    {
    case QLocale::Chinese:
        if (locale.script() == QLocale::TraditionalHanScript)
        {
            return LanguageType::LanguageType_CHINESE_TRADITIONAL;
        }
        return LanguageType::LanguageType_CHINESE_SIMPLIFIED;
    case QLocale::French:
        return LanguageType::LanguageType_FRENCH;
    case QLocale::German:
        return LanguageType::LanguageType_GERMAN;
    case QLocale::Italian:
        return LanguageType::LanguageType_ITALIAN;
    case QLocale::Spanish:
        return LanguageType::LanguageType_SPANISH;
    case QLocale::Portuguese:
        return LanguageType::LanguageType_PORTUGUESE;
    case QLocale::Japanese:
        return LanguageType::LanguageType_JAPANESE;
    case QLocale::Korean:
        return LanguageType::LanguageType_KOREAN;
    case QLocale::Russian:
        return LanguageType::LanguageType_RUSSIAN;
    case QLocale::English:
    default:
        return LanguageType::LanguageType_ENGLISH;
    }
}

} // namespace

std::unique_ptr<ITranslatorManager> ITranslatorManager::createInstance(
    QPointer<UIAppCore::UIApplication> application,
    QPointer<UIAppCore::UIQmlEngine> qmlEngine)
{
    return std::make_unique<TranslatorManager>(application, qmlEngine);
}

TranslatorManager::TranslatorManager(
    QPointer<UIAppCore::UIApplication> application,
    QPointer<UIAppCore::UIQmlEngine> qmlEngine)
    : mApplication(application)
    , mQmlEngine(qmlEngine)
{
    TranslatorManager_LOG_DEBUG("create TranslatorManager");
}

TranslatorManager::~TranslatorManager()
{
    if (mApplication && mTranslator &&
        !mApplication->removeTranslator(mTranslator.get()))
    {
        TranslatorManager_LOG_WARN(
            "failed to remove active translator during destruction, language:"
            << mCurrentLanguage.toStdString());
    }
}

TranslationLoadResult TranslatorManager::loadTranslation(LanguageType languageType)
{
    const auto languageTag = getLanguageTag(languageType);
    if (!languageTag || !isPackagedLanguage(*languageTag))
    {
        TranslatorManager_LOG_WARN(
            "unsupported translation language, type:"
            << static_cast<int>(languageType)
            << ", currentLanguage:" << mCurrentLanguage.toStdString());
        return TranslationLoadResult::UnsupportedLanguage;
    }

    if (!mApplication)
    {
        TranslatorManager_LOG_ERROR(
            "cannot apply translation because UIApplication is unavailable, language:"
            << std::string(*languageTag));
        return TranslationLoadResult::ApplicationUnavailable;
    }

    const auto language = toQString(*languageTag);
    if (language == mCurrentLanguage)
    {
        TranslatorManager_LOG_DEBUG(
            "translation already active, language:" << language.toStdString());
        return TranslationLoadResult::AlreadyActive;
    }

    if (languageType == LanguageType::LanguageType_ENGLISH)
    {
        return applySourceLanguage(language);
    }
    return applyTranslatedLanguage(language);
}

TranslationLoadResult TranslatorManager::loadSystemTranslation()
{
    auto languageType = getSystemLanguageType(QLocale::system());
    const auto languageTag = getLanguageTag(languageType);
    if (!languageTag || !isPackagedLanguage(*languageTag))
    {
        TranslatorManager_LOG_INFO(
            "system language is not packaged; falling back to English");
        languageType = LanguageType::LanguageType_ENGLISH;
    }
    return loadTranslation(languageType);
}

std::vector<LanguageType> TranslatorManager::getAvailableLanguages() const
{
    std::vector<LanguageType> result;
    result.reserve(
        UIResourceTranslation::detail::kPackagedTranslationLocales.size());

    for (const auto locale :
         UIResourceTranslation::detail::kPackagedTranslationLocales)
    {
        const auto languageType = getLanguageType(locale);
        if (!languageType)
        {
            TranslatorManager_LOG_ERROR(
                "packaged translation locale has no LanguageType mapping, locale:"
                << std::string(locale));
            continue;
        }
        result.push_back(*languageType);
    }
    return result;
}

TranslationLoadResult TranslatorManager::applySourceLanguage(const QString& language)
{
    if (mTranslator)
    {
        if (!mApplication->removeTranslator(mTranslator.get()))
        {
            TranslatorManager_LOG_WARN(
                "active translator was not installed while switching to source language, currentLanguage:"
                << mCurrentLanguage.toStdString());
        }
        mTranslator.reset();
    }

    finishLanguageChange(language);
    TranslatorManager_LOG_INFO(
        "source language applied, language:" << language.toStdString());
    return TranslationLoadResult::Applied;
}

TranslationLoadResult TranslatorManager::applyTranslatedLanguage(const QString& language)
{
    const auto resource = getTranslationResource(language);
    auto candidate = std::make_unique<QTranslator>();
    if (!candidate->load(resource))
    {
        TranslatorManager_LOG_WARN(
            "translation resource load failed, requestedLanguage:"
            << language.toStdString()
            << ", currentLanguage:" << mCurrentLanguage.toStdString()
            << ", resource:" << resource.toStdString());
        return TranslationLoadResult::ResourceLoadFailed;
    }

    // The current translator remains installed until the candidate has been
    // loaded and installed successfully.
    if (!mApplication->installTranslator(candidate.get()))
    {
        TranslatorManager_LOG_ERROR(
            "translation install failed, requestedLanguage:"
            << language.toStdString()
            << ", currentLanguage:" << mCurrentLanguage.toStdString()
            << ", resource:" << resource.toStdString());
        return TranslationLoadResult::InstallFailed;
    }

    if (mTranslator && !mApplication->removeTranslator(mTranslator.get()))
    {
        TranslatorManager_LOG_WARN(
            "previous translator was not installed, previousLanguage:"
            << mCurrentLanguage.toStdString()
            << ", newLanguage:" << language.toStdString());
    }

    mTranslator = std::move(candidate);
    finishLanguageChange(language);

    TranslatorManager_LOG_INFO(
        "translation applied, language:" << language.toStdString()
        << ", resource:" << resource.toStdString());
    return TranslationLoadResult::Applied;
}

void TranslatorManager::finishLanguageChange(const QString& language)
{
    mCurrentLanguage = language;
    if (mQmlEngine)
    {
        mQmlEngine->setUiLanguage(language);
        mQmlEngine->retranslate();
    }
    emit languageChanged(language);
}

} // namespace UIManager
