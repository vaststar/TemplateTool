#pragma once

#include <QObject>
#include <QPointer>
#include <QtQml>
#include <UICore/CoreController.h>

class AppContext;
class MainWindowFootBarController: public UICore::CoreController
{
    Q_OBJECT
    Q_PROPERTY(QString footerName READ getFooterName NOTIFY footerNameChanged)
    QML_ELEMENT
public:
    explicit MainWindowFootBarController(QObject *parent = nullptr);
    void initializeController(QPointer<AppContext> appContext);
    QString getFooterName() const;
signals:
    void footerNameChanged();
private:
    QPointer<AppContext> mAppContext;
};