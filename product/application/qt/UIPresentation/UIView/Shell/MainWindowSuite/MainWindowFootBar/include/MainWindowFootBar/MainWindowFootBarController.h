#pragma once

#include <QObject>
#include <QPointer>
#include <QtQml>
#include <UIViewCore/UIViewController.h>

class MainWindowFootBarController: public UIViewController
{
    Q_OBJECT
    Q_PROPERTY(QString footerName READ getFooterName NOTIFY footerNameChanged)
    QML_ELEMENT
public:
    explicit MainWindowFootBarController(QObject *parent = nullptr);
    ~MainWindowFootBarController() override;
    QString getFooterName() const;
protected:
    virtual void init() override;
signals:
    void footerNameChanged();
};
