#pragma once

#include <QObject>
#include <QPointer>
#include <QtQml>
#include <UICore/CoreController.h>

class AppContext;
class AppSystemTrayController: public UICore::CoreController
{
    Q_OBJECT
    Q_PROPERTY(QString title READ getTitle NOTIFY titleChanged)
    QML_ELEMENT
public:
    explicit AppSystemTrayController(QObject *parent = nullptr);
    virtual QString getControllerName() const override;
    void initializeController(QPointer<AppContext> appContext);
    QString getTitle() const;
signals:
    void titleChanged();
private:
    QPointer<AppContext> mAppContext;
};