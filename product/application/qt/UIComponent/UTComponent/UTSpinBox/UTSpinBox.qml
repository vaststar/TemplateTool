import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTSpinBox - Themed spin box with UT design tokens.
 *
 * Features:
 * - Themed background, text, and +/- buttons
 * - Rounded border matching other UT input controls
 * - Smooth hover/press color transitions
 * - Focus ring via UTFocusItem
 *
 * Usage:
 *   UTSpinBox {
 *       from: 0; to: 100; value: 4
 *       onValueChanged: myModel.count = value
 *   }
 */
BaseSpinBox {
    id: control

    property var backgroundColorEnum: UIColorToken.Spinbox_Background
    property var textColorEnum: UIColorToken.Spinbox_Text
    property var buttonColorEnum: UIColorToken.Spinbox_Button
    property var borderColorEnum: UIColorToken.Content_Input_Border

    font: UTComponentUtil.getUIFont(UIFontToken.Body_Text)
    backgroundColor: UTComponentUtil.getPlainUIColor(
                         backgroundColorEnum,
                         control.enabled ? UIColorState.Normal : UIColorState.Disabled)
    textColor: UTComponentUtil.getPlainUIColor(
                   textColorEnum,
                   control.enabled ? UIColorState.Normal : UIColorState.Disabled)
    upButtonColor: {
        if (!control.enabled)
            return UTComponentUtil.getPlainUIColor(control.buttonColorEnum, UIColorState.Disabled)
        if (control.up.pressed)
            return UTComponentUtil.getPlainUIColor(control.buttonColorEnum, UIColorState.Pressed)
        if (control.up.hovered)
            return UTComponentUtil.getPlainUIColor(control.buttonColorEnum, UIColorState.Hovered)
        return UTComponentUtil.getPlainUIColor(control.buttonColorEnum, UIColorState.Normal)
    }
    downButtonColor: {
        if (!control.enabled)
            return UTComponentUtil.getPlainUIColor(control.buttonColorEnum, UIColorState.Disabled)
        if (control.down.pressed)
            return UTComponentUtil.getPlainUIColor(control.buttonColorEnum, UIColorState.Pressed)
        if (control.down.hovered)
            return UTComponentUtil.getPlainUIColor(control.buttonColorEnum, UIColorState.Hovered)
        return UTComponentUtil.getPlainUIColor(control.buttonColorEnum, UIColorState.Normal)
    }
    borderColor: UTComponentUtil.getPlainUIColor(
                     borderColorEnum,
                     control.activeFocus ? UIColorState.Focused : UIColorState.Normal)
    selectionColor: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border, UIColorState.Focused)
    selectedTextColor: UTComponentUtil.getPlainUIColor(textColorEnum, UIColorState.Normal)

    UTFocusItem {
        focusRadius: control.borderRadius
    }
}
