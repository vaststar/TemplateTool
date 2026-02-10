#pragma once

#include <memory>
#include <QObject>
#include <QString>
#include <QTranslator>

#include <UIManager/UILanguage.h>
#include <UICore/CoreApplication.h>
#include <UICore/CoreQmlEngine.h>

#include <UIManager/ITranslatorManager.h>

namespace UIManager{
class TranslatorManager final: public ITranslatorManager
{
Q_OBJECT
public:
    explicit TranslatorManager(QPointer<UICore::CoreApplication> application, QPointer<UICore::CoreQmlEngine> qmlEngine);
    ~TranslatorManager();
    TranslatorManager(const TranslatorManager&) = delete;
    TranslatorManager(TranslatorManager&&) = delete;
    TranslatorManager& operator=(const TranslatorManager&) = delete;
    TranslatorManager& operator=(TranslatorManager&&) = delete;

    virtual void loadSystemTranslation() override;
    // 加载特定语言的翻译文件
    virtual void loadTranslation(UILanguage::LanguageType languageType) override;
private:
    void loadTranslation(const QString& language);
    std::string getLanguageString(UILanguage::LanguageType languageType) const;
    std::map<UILanguage::LanguageType, std::string> getLanguageMap() const;
private:
    const QPointer<UICore::CoreApplication> mApplication;
    const QPointer<UICore::CoreQmlEngine> mQmlEngine;
    const std::unique_ptr<QTranslator> mTranslator;

    QString mCurrentLanguage;
};
}
