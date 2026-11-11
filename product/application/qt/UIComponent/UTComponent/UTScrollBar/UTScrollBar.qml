import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTScrollBar - Themed ScrollBar with arrow buttons and UT design tokens.
 *
 * Features:
 * - Narrow track with rounded thumb
 * - Top/bottom (or left/right) arrow buttons for step scrolling
 * - Hover/pressed visual feedback with smooth animations
 * - showArrows property to toggle arrow buttons
 *
 * Usage:
 *   ListView {
 *       ScrollBar.vertical: UTScrollBar {}
 *   }
 */
BaseScrollBar {
    id: control

    property var trackColorEnum: UIColorToken.Scrollbar_Track
    property var thumbColorEnum: UIColorToken.Scrollbar_Thumb
    property var arrowColorEnum: UIColorToken.Scrollbar_Arrow

    trackColor: UTComponentUtil.getPlainUIColor(control.trackColorEnum, UIColorState.Normal)
    thumbColor: UTComponentUtil.getItemUIColor(control, control.thumbColorEnum)
    decreaseArrowColor: UTComponentUtil.getPlainUIColor(control.arrowColorEnum,
                           control.decreaseHovered ? UIColorState.Hovered : UIColorState.Normal)
    increaseArrowColor: UTComponentUtil.getPlainUIColor(control.arrowColorEnum,
                           control.increaseHovered ? UIColorState.Hovered : UIColorState.Normal)
}
