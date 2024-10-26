#pragma once

#include <QString>
#include <QQmlApplicationEngine>
#include <UICore/UICoreExport.h>

class UICore_EXPORT CoreQmlApplicationEngine: public QQmlApplicationEngine
{
Q_OBJECT
public:
    CoreQmlApplicationEngine(QObject* parent = nullptr);
    CoreQmlApplicationEngine(const QUrl& url, QObject* parent = nullptr);
    // QString getDefaultQRCPrefix() const;
};