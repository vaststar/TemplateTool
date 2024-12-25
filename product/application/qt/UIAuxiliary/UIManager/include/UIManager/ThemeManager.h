#pragma once

#include <memory>
#include <QObject>
#include <QString>
#include <QtQml>

#include <UIManager/UIManagerExport.h>

namespace UICore{
    class CoreApplication;
    class CoreQmlEngine;
}

namespace UIManager{
class UIManager_EXPORT ThemeManager final: public QObject
{
Q_OBJECT
QML_ELEMENT
public:
    explicit ThemeManager(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine);
    ~ThemeManager();

public slots:
    void test();
private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
};
}
