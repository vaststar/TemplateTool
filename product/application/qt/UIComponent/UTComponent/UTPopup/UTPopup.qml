import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTPopup - Themed popup with design-token background and border.
 *
 * Usage:
 *   UTPopup {
 *       y: parent.height + 2; width: 160; padding: 4
 *       Column { ... }
 *   }
 */
BasePopup {
    id: control

    property var backgroundColorEnum: UIColorToken.Popup_Background
    property var borderColorEnum: UIColorToken.Popup_Border
    property int borderRadius: 4

    background: Rectangle {
        color: UTComponentUtil.getPlainUIColor(control.backgroundColorEnum, UIColorState.Normal)
        border.color: UTComponentUtil.getPlainUIColor(control.borderColorEnum, UIColorState.Normal)
        border.width: 1
        radius: control.borderRadius
    }
}
