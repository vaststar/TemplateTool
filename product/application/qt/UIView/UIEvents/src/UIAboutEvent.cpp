#include "UIEvents/UIAboutEvent.h"

QEvent::Type UIAboutEvent::eventType()
{
    static const auto type = static_cast<QEvent::Type>(QEvent::registerEventType());
    return type;
}

UIAboutEvent::UIAboutEvent(Action action)
    : UIManager::UIEvent<UIAboutEvent>(eventType())
    , mAction(action)
{
}
