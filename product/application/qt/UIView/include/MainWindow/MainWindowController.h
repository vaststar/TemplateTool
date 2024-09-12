#pragma once

#include <QString>
#include <QObject>
#include <QtQml>
#include <UICore/CoreController.h>
#include <UICore/CoreContext.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}

class CoreContext;
class MainWindowController : public CoreController
{
    Q_OBJECT
    Q_PROPERTY(QString mTitle READ getTitle NOTIFY titleChanged)
    QML_ELEMENT
public:
    MainWindowController(QObject* parent =nullptr);
    virtual QString getControllerName() const override;

    QString getTitle() const;

    void showMainWindow();
    void initializeController(CoreContext* appContext);
signals:
    void titleChanged();
    void controllerInitialized(CoreContext*);
private:
    CoreContext* mAppContext;
    QString mTitle;
};