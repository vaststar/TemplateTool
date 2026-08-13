#include "UIEvents/UIScreenChangedEvent.h"

QEvent::Type UIScreenChangedEvent::eventType()
{
    static const auto type = static_cast<QEvent::Type>(QEvent::registerEventType());
    return type;
}

UIScreenChangedEvent::UIScreenChangedEvent()
    : UIManager::UIEvent<UIScreenChangedEvent>(eventType())
{
}
