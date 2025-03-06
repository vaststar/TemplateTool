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

class UIManager_EXPORT ITranslatorManager: public QObject
{
Q_OBJECT
public:
    virtual ~ITranslatorManager() = default;

    virtual void loadSystemTranslation() = 0;
    // 加载特定语言的翻译文件
    virtual void loadTranslation(UILanguage::LanguageType languageType) = 0;
signals:
    void languageChanged(const QString& language);
};
}
