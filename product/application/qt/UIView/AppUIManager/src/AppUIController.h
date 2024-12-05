#pragma once

#include <QString>
#include <QObject>
#include <QtQml>
#include <UICore/CoreController.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}

class AppContext;
class AppUIController : public UICore::CoreController
{
    Q_OBJECT
    // Q_PROPERTY(MenuItemModel* rootMenu READ rootMenu CONSTANT)
    // QML_ELEMENT
public:
    AppUIController(QObject* parent = nullptr);
    virtual QString getControllerName() const override;

    void initializeController(AppContext* appContext);

    void startApp();
signals:
    void controllerInitialized();
private:
    QPointer<AppContext> mAppContext;
};