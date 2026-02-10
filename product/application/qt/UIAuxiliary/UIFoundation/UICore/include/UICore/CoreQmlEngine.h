#pragma once

#include <QString>
#include <QQmlApplicationEngine>
#include <UICore/UICoreExport.h>


namespace UICore{
class UICore_EXPORT CoreQmlEngine: public QQmlApplicationEngine
{
Q_OBJECT
public:
    CoreQmlEngine(QObject* parent = nullptr);
    CoreQmlEngine(const QUrl& url, QObject* parent = nullptr);
    CoreQmlEngine(const CoreQmlEngine&) = delete;
    CoreQmlEngine(CoreQmlEngine&&) = delete;
    CoreQmlEngine& operator=(const CoreQmlEngine&) = delete;
    CoreQmlEngine& operator=(CoreQmlEngine&&) = delete;
    virtual ~CoreQmlEngine() = default;
};
}