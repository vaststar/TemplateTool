import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTSlider - Themed horizontal slider with UT design tokens.
 *
 * Features:
 * - Themed track (groove), filled progress, and round handle
 * - Smooth hover/press color transitions
 * - Focus ring via UTFocusItem
 *
 * Usage:
 *   UTSlider {
 *       from: 0; to: 100; value: 50
 *       onValueChanged: console.log(value)
 *   }
 */
BaseSlider {
    id: control

    property var trackColorEnum: UIColorToken.Slider_Track
    property var progressColorEnum: UIColorToken.Slider_Progress
    property var handleColorEnum: UIColorToken.Slider_Handle
    // The handle fill is intentionally light (near-white in the light theme,
    // mirroring switch-thumb). On a light background a borderless white circle
    // is invisible, so delineate it with a themed border.
    property var handleBorderColorEnum: UIColorToken.Content_Input_Border

    trackColor: UTComponentUtil.getPlainUIColor(trackColorEnum, UIColorState.Normal)
    progressColor: UTComponentUtil.getPlainUIColor(
                       progressColorEnum,
                       control.enabled ? UIColorState.Normal : UIColorState.Disabled)
    handleColor: {
        if (!control.enabled)
            return UTComponentUtil.getPlainUIColor(control.handleColorEnum, UIColorState.Disabled)
        if (control.pressed)
            return UTComponentUtil.getPlainUIColor(control.handleColorEnum, UIColorState.Pressed)
        if (control.hovered)
            return UTComponentUtil.getPlainUIColor(control.handleColorEnum, UIColorState.Hovered)
        return UTComponentUtil.getPlainUIColor(control.handleColorEnum, UIColorState.Normal)
    }
    handleBorderColor: UTComponentUtil.getPlainUIColor(
                           handleBorderColorEnum,
                           control.enabled ? UIColorState.Normal : UIColorState.Disabled)

    UTFocusItem {
        focusRadius: control.handleSize / 2
    }
}
