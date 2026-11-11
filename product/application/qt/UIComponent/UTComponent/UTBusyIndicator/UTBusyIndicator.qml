import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTBusyIndicator - Themed indeterminate spinner with UT design tokens.
 *
 * Features:
 * - Track ring + orbiting dot, both colored from theme tokens
 * - Configurable size, ring thickness, dot size and spin cycle
 *
 * Usage:
 *   UTBusyIndicator { running: someLoadingFlag }
 *   UTBusyIndicator { indicatorSize: 48; cycleDuration: 1200 }
 */
BaseBusyIndicator {
    id: control

    property var trackColorEnum: UIColorToken.Busy_Indicator_Track
    property var progressColorEnum: UIColorToken.Busy_Indicator_Progress

    trackColor: UTComponentUtil.getPlainUIColor(
                    control.trackColorEnum,
                    control.enabled ? UIColorState.Normal : UIColorState.Disabled)
    progressColor: UTComponentUtil.getPlainUIColor(
                       control.progressColorEnum,
                       control.enabled ? UIColorState.Normal : UIColorState.Disabled)
}
