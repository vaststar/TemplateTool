#include "UIEvents/UISettingsEvent.h"

QEvent::Type UISettingsEvent::eventType()
{
    static const auto type = static_cast<QEvent::Type>(QEvent::registerEventType());
    return type;
}

UISettingsEvent::UISettingsEvent(Action action)
    : UIManager::UIEvent<UISettingsEvent>(eventType())
    , mAction(action)
{
}
