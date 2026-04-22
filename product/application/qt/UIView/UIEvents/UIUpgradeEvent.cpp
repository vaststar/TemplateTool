#include "UIEvents/UIUpgradeEvent.h"

UIUpgradeEvent::UIUpgradeEvent(Action action)
    : UIManager::UIEvent<UIUpgradeEvent>()
    , mAction(action)
{
}
