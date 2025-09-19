#pragma once

#include <QObject>
#include <QPointer>
#include <QtQml>
#include <UICore/CoreController.h>

class AppContext;
class MainWindowSideBarController: public UICore::CoreController
{
    Q_OBJECT
    Q_PROPERTY(QString title READ getTitle NOTIFY titleChanged)
    QML_ELEMENT
public:
    explicit MainWindowSideBarController(QObject *parent = nullptr);
    void initializeController(QPointer<AppContext> appContext);
    QString getTitle() const;
signals:
    void titleChanged();
private:
    QPointer<AppContext> mAppContext;
};