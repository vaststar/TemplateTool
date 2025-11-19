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
    CoreController(const CoreController&) = delete;
    CoreController(CoreController&&) = delete;
    CoreController& operator=(const CoreController&) = delete;
    CoreController& operator=(CoreController&&) = delete;
    virtual ~CoreController() = default;
public:
    QString getControllerName() const;
signals:
    void controllerNameChanged();
};
}