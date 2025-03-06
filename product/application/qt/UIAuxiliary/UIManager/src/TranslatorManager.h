#pragma once

#include <memory>
#include <QObject>
#include <QString>

#include <UIManager/ITranslatorManager.h>
#include <UIDataStruct/UILanguage.h>

namespace UICore{
    class CoreApplication;
    class CoreQmlEngine;
}


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
private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
};
}
