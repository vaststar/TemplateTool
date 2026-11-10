import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTCheckBox - Themed checkbox with UT design tokens.
 *
 * Features:
 * - Rounded-rect indicator with checkmark
 * - Theme-aware colors via UIColorToken (light/dark)
 * - Smooth color animations
 * - Optional text label
 * - Focus ring via UTFocusItem
 *
 * Usage:
 *   UTCheckBox {
 *       text: qsTr("Enable feature")
 *       checked: myModel.enabled
 *       onToggled: myModel.enabled = checked
 *   }
 *
 *   // Without text (toggle-only)
 *   UTCheckBox {
 *       checked: controller.flag
 *       onToggled: controller.flag = checked
 *   }
 */
BaseCheckBox {
    id: control

    property var boxColorEnum: UIColorToken.Checkbox_Box
    property var checkColorEnum: UIColorToken.Checkbox_Check
    property var textColorEnum: UIColorToken.Checkbox_Text
    property var borderColorEnum: UIColorToken.Content_Input_Border

    textFont: UTComponentUtil.getUIFont(UIFontToken.Body_Text)
    boxColor: {
        if (!control.enabled)
            return UTComponentUtil.getPlainUIColor(control.boxColorEnum, UIColorState.Disabled)
        if (control.checked)
            return UTComponentUtil.getPlainUIColor(control.boxColorEnum, UIColorState.Checked)
        if (control.pressed)
            return UTComponentUtil.getPlainUIColor(control.boxColorEnum, UIColorState.Pressed)
        if (control.hovered)
            return UTComponentUtil.getPlainUIColor(control.boxColorEnum, UIColorState.Hovered)
        return UTComponentUtil.getPlainUIColor(control.boxColorEnum, UIColorState.Normal)
    }
    checkColor: UTComponentUtil.getPlainUIColor(checkColorEnum, UIColorState.Normal)
    textColor: UTComponentUtil.getPlainUIColor(textColorEnum, UIColorState.Normal)
    borderColor: UTComponentUtil.getPlainUIColor(
                     borderColorEnum,
                     control.activeFocus ? UIColorState.Focused : UIColorState.Normal)

    UTFocusItem {
        focusRadius: control.boxRadius
    }
}
