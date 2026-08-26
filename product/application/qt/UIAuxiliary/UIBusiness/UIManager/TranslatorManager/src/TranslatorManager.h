#pragma once

#include <memory>
#include <vector>

#include <QObject>
#include <QPointer>
#include <QString>
#include <QTranslator>

#include <TranslatorManager/UILanguage.h>
#include <UIAppCore/UIApplication.h>
#include <UIAppCore/UIQmlEngine.h>

#include <TranslatorManager/ITranslatorManager.h>

namespace UIManager{
class TranslatorManager final: public ITranslatorManager
{
Q_OBJECT
public:
    explicit TranslatorManager(QPointer<UIAppCore::UIApplication> application, QPointer<UIAppCore::UIQmlEngine> qmlEngine);
    ~TranslatorManager();
    TranslatorManager(const TranslatorManager&) = delete;
    TranslatorManager(TranslatorManager&&) = delete;
    TranslatorManager& operator=(const TranslatorManager&) = delete;
    TranslatorManager& operator=(TranslatorManager&&) = delete;

    [[nodiscard]] TranslationLoadResult loadSystemTranslation() override;
    // 加载特定语言的翻译文件
    [[nodiscard]] TranslationLoadResult loadTranslation(UILanguage::LanguageType languageType) override;
    [[nodiscard]] std::vector<UILanguage::LanguageType> getAvailableLanguages() const override;
private:
    [[nodiscard]] TranslationLoadResult applySourceLanguage(const QString& language);
    [[nodiscard]] TranslationLoadResult applyTranslatedLanguage(const QString& language);
    void finishLanguageChange(const QString& language);
private:
    const QPointer<UIAppCore::UIApplication> mApplication;
    const QPointer<UIAppCore::UIQmlEngine> mQmlEngine;
    // nullptr means that the source language (English) is active.
    std::unique_ptr<QTranslator> mTranslator;

    QString mCurrentLanguage;
};
}
