#pragma once
#include <QString>
#include <QtQml>
#include <UICore/CoreController.h>

class TestController:public CoreController
{
Q_OBJECT
    // Q_PROPERTY(QString mControllerName READ getControllerName NOTIFY controllerNameChanged)
QML_ELEMENT
public:
    TestController(QObject* parent = nullptr);
public:
    void setName();
    
    virtual QString getControllerName() const ;
// signals:
    // void controllerNameChanged();
private:
    QString name;
};