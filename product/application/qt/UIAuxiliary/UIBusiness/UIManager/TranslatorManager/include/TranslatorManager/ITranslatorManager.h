#pragma once

#include <memory>
#include <QObject>
#include <QString>

#include <TranslatorManager/TranslatorManagerExport.h>
#include <TranslatorManager/UILanguage.h>

namespace UICore{
    class CoreApplication;
    class CoreQmlEngine;
}


namespace UIManager{

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
    virtual void loadSystemTranslation() = 0;
    // 加载特定语言的翻译文件
    virtual void loadTranslation(UILanguage::LanguageType languageType) = 0;

    static std::unique_ptr<ITranslatorManager> createInstance(QPointer<QObject> application, QPointer<QObject> qmlEngine);
signals:
    void languageChanged(const QString& language);
};
}
