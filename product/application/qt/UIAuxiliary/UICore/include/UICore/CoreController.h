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
    QString getControllerName() const;
signals:
    void controllerNameChanged();
};
}