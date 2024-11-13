#pragma once

#include <memory>
#include <QObject>
#include <QString>

#include <UIManager/UIManagerExport.h>

namespace UICore{
    class CoreApplication;
    class CoreQmlEngine;
}

namespace UIManager{
Q_NAMESPACE
enum class LanguageType {
    ENGLISH,
    CHINESE_SIMPLIFIED,
    CHINESE_TRADITIONAL,
    FRENCH,
    GERMAN,
    ITALIAN,
    SPANISH,
    PORTUGUESE,
    JAPANESE,
    KOREAN,
    RUSSIAN
};
Q_ENUM_NS(LanguageType)

class UIManager_EXPORT TranslatorManager final: public QObject
{
Q_OBJECT
public:
    explicit TranslatorManager(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine);
    ~TranslatorManager();

    void loadSystemTranslation();
    // 加载特定语言的翻译文件
    void loadTranslation(LanguageType languageType);
signals:
    void languageChanged(const QString& language);
private:
    void loadTranslation(const QString& language);
private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
};
}
