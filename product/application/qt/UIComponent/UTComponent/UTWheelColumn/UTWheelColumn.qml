import QtQuick
import QtQuick.Controls.Basic
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTWheelColumn - Themed, interactive wheel column over BaseWheelColumn.
 *
 * Adds the themed delegate, focus ring, tap-to-activate and wheel stepping.
 * Shared by UTTimePicker and UTDateWheelPicker.
 */
BaseWheelColumn {
    id: wheel

    property var textColorEnum: UIColorToken.Timepicker_Text
    property var borderColorEnum: UIColorToken.Timepicker_Border
    property real borderRadius: 6
    // Maps an index to its display string. Default: zero-padded 2-digit number.
    property var formatValue: function(i) { return (i < 10 ? "0" : "") + i }

    // Focus ring tracks real keyboard focus (gated on keyboard navigation by
    // BaseFocusItem); no external flag needed.
    UTFocusItem {
        target: wheel
        delegateFocused: wheel.activeFocus
        focusRadius: wheel.borderRadius - 2
    }

    TapHandler {
        onTapped: wheel.forceActiveFocus()
    }

    // NoButton consumes wheel events (so the Tumbler's flickable doesn't also
    // flick) while letting press/drag pass through.
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.NoButton
        onWheel: function(event) {
            wheel.stepBy(event.angleDelta.y > 0 ? -1 : 1)
            event.accepted = true
        }
    }

    delegate: Component {
        UTText {
            required property int index
            readonly property bool disabled: index >= wheel.enabledCount
            text: wheel.formatValue(index)
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: UTComponentUtil.getPlainUIColor(
                       wheel.textColorEnum,
                       disabled ? UIColorState.Disabled : UIColorState.Normal)
            opacity: disabled
                     ? 0.35
                     : 1.0 - Math.min(0.6, Math.abs(Tumbler.displacement) * 0.28)
            font.pixelSize: Tumbler.displacement === 0 ? 18 : 14
            font.bold: !disabled && Tumbler.displacement === 0
        }
    }
}
