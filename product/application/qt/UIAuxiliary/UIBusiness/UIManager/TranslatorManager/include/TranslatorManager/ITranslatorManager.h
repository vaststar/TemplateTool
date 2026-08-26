#pragma once

#include <memory>
#include <vector>

#include <QObject>
#include <QPointer>
#include <QString>

#include <TranslatorManager/TranslatorManagerExport.h>
#include <TranslatorManager/UILanguage.h>

namespace UIAppCore{
    class UIApplication;
    class UIQmlEngine;
}


namespace UIManager{

enum class TranslationLoadResult
{
    Applied,
    AlreadyActive,
    UnsupportedLanguage,
    ApplicationUnavailable,
    ResourceLoadFailed,
    InstallFailed
};

[[nodiscard]] constexpr bool isTranslationLoadSuccessful(
    TranslationLoadResult result) noexcept
{
    return result == TranslationLoadResult::Applied ||
           result == TranslationLoadResult::AlreadyActive;
}

class TranslatorManager_EXPORT ITranslatorManager: public QObject
{
Q_OBJECT
public:
    ITranslatorManager() = default;
    ITranslatorManager(const ITranslatorManager&) = delete;
    ITranslatorManager(ITranslatorManager&&) = delete;
    ITranslatorManager& operator=(const ITranslatorManager&) = delete;
    ITranslatorManager& operator=(ITranslatorManager&&) = delete;
    virtual ~ITranslatorManager() = default;
public:
    [[nodiscard]] virtual TranslationLoadResult loadSystemTranslation() = 0;
    // 加载特定语言的翻译文件
    [[nodiscard]] virtual TranslationLoadResult loadTranslation(UILanguage::LanguageType languageType) = 0;
    [[nodiscard]] virtual std::vector<UILanguage::LanguageType> getAvailableLanguages() const = 0;

    static std::unique_ptr<ITranslatorManager> createInstance(QPointer<UIAppCore::UIApplication> application, QPointer<UIAppCore::UIQmlEngine> qmlEngine);
signals:
    void languageChanged(const QString& language);
};
}
