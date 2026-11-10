import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTSwitch - Themed toggle switch with UT design tokens.
 *
 * Features:
 * - Rounded track with sliding circular thumb
 * - Theme-aware colors via UIColorToken (light/dark)
 * - Smooth position and color animations
 * - Focus ring via UTFocusItem
 *
 * Usage:
 *   UTSwitch {
 *       checked: myModel.enabled
 *       onToggled: myModel.enabled = checked
 *   }
 */
BaseSwitch {
    id: control

    property var trackColorEnum: UIColorToken.Switch_Track
    property var thumbColorEnum: UIColorToken.Switch_Thumb

    trackColor: {
        if (!control.enabled)
            return UTComponentUtil.getPlainUIColor(control.trackColorEnum, UIColorState.Disabled)
        if (control.checked)
            return UTComponentUtil.getPlainUIColor(control.trackColorEnum, UIColorState.Checked)
        if (control.pressed)
            return UTComponentUtil.getPlainUIColor(control.trackColorEnum, UIColorState.Pressed)
        if (control.hovered)
            return UTComponentUtil.getPlainUIColor(control.trackColorEnum, UIColorState.Hovered)
        return UTComponentUtil.getPlainUIColor(control.trackColorEnum, UIColorState.Normal)
    }
    thumbColor: UTComponentUtil.getPlainUIColor(
                    thumbColorEnum,
                    control.enabled ? UIColorState.Normal : UIColorState.Disabled)

    UTFocusItem {
        focusRadius: control.trackHeight / 2
    }
}
