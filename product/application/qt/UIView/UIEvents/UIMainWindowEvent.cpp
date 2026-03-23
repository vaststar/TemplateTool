#include "UIEvents/UIMainWindowEvent.h"

UIMainWindowEvent::UIMainWindowEvent(Action action)
    : UIManager::UIEvent<UIMainWindowEvent>()
    , mAction(action)
{
}
