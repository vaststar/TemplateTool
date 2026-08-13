#include "UIEvents/UIUpgradeEvent.h"

QEvent::Type UIUpgradeEvent::eventType()
{
    static const auto type = static_cast<QEvent::Type>(QEvent::registerEventType());
    return type;
}

UIUpgradeEvent::UIUpgradeEvent(Action action)
    : UIManager::UIEvent<UIUpgradeEvent>(eventType())
    , mAction(action)
{
}
