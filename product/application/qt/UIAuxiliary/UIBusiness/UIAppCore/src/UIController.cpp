#include <UIAppCore/UIController.h>

namespace UIAppCore {

UIController::UIController(QObject* parent)
    : UICore::CoreController(parent)
{
}

QString UIController::getControllerName() const
{
    return metaObject()->className();
}

}
