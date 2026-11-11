import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTProgressBar - Themed progress bar with UT design tokens.
 *
 * Features:
 * - Themed track and progress fill using design tokens
 * - Smooth color transitions based on enabled state
 * - Customizable height and colors
 *
 * Usage:
 *   UTProgressBar {
 *       from: 0; to: 1.0; value: 0.5
 *   }
 */
BaseProgressBar {
    id: control

    property var trackColorEnum: UIColorToken.Slider_Track
    property var progressColorEnum: UIColorToken.Slider_Progress

    trackColor: UTComponentUtil.getPlainUIColor(control.trackColorEnum, UIColorState.Normal)
    progressColor: UTComponentUtil.getPlainUIColor(
                       control.progressColorEnum,
                       control.enabled ? UIColorState.Normal : UIColorState.Disabled)
}
