#pragma once

#include <QObject>

#include <UIManager/UIManagerExport.h>
namespace UICore{
    class CoreApplication;
    class CoreQmlEngine;
}

namespace UIManager{
class TranslatorManager;
class UIManager_EXPORT UIManagerProvider final: public QObject
{
Q_OBJECT
public:
    explicit UIManagerProvider(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine);
    ~UIManagerProvider();
    QPointer<TranslatorManager> getTranslatorManager() const;
private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
};
}
