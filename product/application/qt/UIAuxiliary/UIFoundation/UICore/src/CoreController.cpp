#include <UICore/CoreController.h>


namespace UICore{
CoreController::CoreController(QObject* parent)
    :QObject(parent)
{

}

QString CoreController::getControllerName() const
{
    return metaObject()->className();
}
}