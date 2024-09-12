#include "MainWindow/TestController.h"

TestController::TestController(QObject *parent)
    :CoreController(parent)
{
    name="test";
}


QString TestController::getControllerName() const
{
    return  name;
}

void  TestController::setName()
{
    name = "new name";
    emit controllerNameChanged();
}