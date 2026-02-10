#pragma once
#include <UICore/CoreController.h>
#include <UIAppCore/UIAppCoreExport.h>

namespace UIAppCore {

class UIAppCore_EXPORT UIController : public UICore::CoreController
{
    Q_OBJECT
    Q_PROPERTY(QString mControllerName READ getControllerName NOTIFY controllerNameChanged)
public:
    UIController(QObject* parent = nullptr);
    UIController(const UIController&) = delete;
    UIController(UIController&&) = delete;
    UIController& operator=(const UIController&) = delete;
    UIController& operator=(UIController&&) = delete;
    virtual ~UIController() = default;

    QString getControllerName() const;

signals:
    void controllerNameChanged();
};

}
