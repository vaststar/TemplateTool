#pragma once

#include <QObject>
#include <QPointer>
#include <QtQml>
#include "UIViewBase/include/UIViewController.h"

class AppContext;
class AppSystemTrayController: public UIViewController
{
    Q_OBJECT
    Q_PROPERTY(QString title READ getTitle NOTIFY titleChanged)
    QML_ELEMENT
public:
    explicit AppSystemTrayController(QObject *parent = nullptr);
    QString getTitle() const;
protected:
    void init() override;
signals:
    void titleChanged();
};