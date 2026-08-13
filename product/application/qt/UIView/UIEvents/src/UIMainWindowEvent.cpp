#include "UIEvents/UIMainWindowEvent.h"

QEvent::Type UIMainWindowEvent::eventType()
{
    static const auto type = static_cast<QEvent::Type>(QEvent::registerEventType());
    return type;
}

UIMainWindowEvent::UIMainWindowEvent(Action action)
    : UIManager::UIEvent<UIMainWindowEvent>(eventType())
    , mAction(action)
{
}
