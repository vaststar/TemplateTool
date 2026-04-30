#include "UIEvents/UIAboutEvent.h"

UIAboutEvent::UIAboutEvent(Action action)
    : UIManager::UIEvent<UIAboutEvent>()
    , mAction(action)
{
}
