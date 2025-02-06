#pragma once

#include <memory>
#include <QObject>
#include <QString>

#include <UIManager/UIManagerExport.h>
#include <UIDataStruct/UILanguage.h>

namespace UICore{
    class CoreApplication;
    class CoreQmlEngine;
}


namespace UIManager{

class UIManager_EXPORT TranslatorManager final: public QObject
{
Q_OBJECT
public:
    explicit TranslatorManager(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine);
    ~TranslatorManager();

    void loadSystemTranslation();
    // 加载特定语言的翻译文件
    void loadTranslation(UILanguage::LanguageType languageType);
signals:
    void languageChanged(const QString& language);
private:
    void loadTranslation(const QString& language);
private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
};
}
