#pragma once

#include <QObject>
#include <QPointer>
#include <QtQml>
#include <QColor>

#include "UIViewBase/include/UIViewController.h"

class AppContext;
class MainWindowTitleBarController: public UIViewController
{
    Q_OBJECT
    Q_PROPERTY(QString title READ getTitle NOTIFY elementUpdated)
    Q_PROPERTY(bool visible READ isVisible NOTIFY elementUpdated)
    Q_PROPERTY(QColor color READ getColor NOTIFY elementUpdated)
    QML_ELEMENT
public:
    explicit MainWindowTitleBarController(QObject *parent = nullptr);
public:
    QString getTitle() const;
    bool isVisible() const;
    QColor getColor() const;
signals:
    void elementUpdated();
protected:
    virtual void init() override;
};