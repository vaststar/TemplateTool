#pragma once

#include <QObject>

#include <UICore/UICoreExport.h>

namespace UICore{
class UICore_EXPORT CoreController: public QObject
{
Q_OBJECT
    Q_PROPERTY(QString mControllerName READ getControllerName NOTIFY controllerNameChanged)
public:
    CoreController(QObject *parent = nullptr);
    virtual ~CoreController() = default;
    virtual QString getControllerName() const = 0;
signals:
    void controllerNameChanged();
};
}