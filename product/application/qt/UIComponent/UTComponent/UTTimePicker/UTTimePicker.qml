import QtQuick
import QtQuick.Controls.Basic
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTTimePicker - Themed time picker with scrolling wheels (Tumbler columns).
 *
 * Collapsed: an input-style box showing the current time (e.g. "09:30").
 * Expanded: a drop-down with hour / minute (/ second) wheels. Each column is a
 * Tumbler - scroll, drag or use arrow keys; values wrap around. The centered row
 * is the selection (larger, fully opaque); neighbours fade out for the classic
 * spinning-drum look.
 *
 * Usage:
 *   UTTimePicker {
 *       hours: 9; minutes: 30
 *       onTimeChanged: myModel.time = timeValue
 *   }
 */
BaseTimePicker {
    id: control

    // === Theme tokens ===
    property var backgroundColorEnum: UIColorToken.Timepicker_Background
    property var textColorEnum:       UIColorToken.Timepicker_Text
    property var borderColorEnum:     UIColorToken.Timepicker_Border
    property var columnColorEnum:     UIColorToken.Timepicker_Column_Background

    // === Wheel geometry ===
    property int columnWidth: 56
    property int wheelHeight: 160
    property int visibleRows: 5
    property int wheelSpacing: 6
    property int colonWidth: 10

    // Number of wheel columns / colon separators currently shown.
    readonly property int wheelCount: showSeconds ? 3 : 2
    readonly property int colonCount: wheelCount - 1
    // Column width that makes the wheels fill the popup width evenly.
    readonly property real autoColumnWidth: {
        var inner = popup.availableWidth
        var used = colonCount * colonWidth + wheelSpacing * (wheelCount + colonCount - 1)
        return Math.max(36, (inner - used) / wheelCount)
    }

    implicitWidth: 180
    implicitHeight: 32
    padding: 0

    font: UTComponentUtil.getUIFont(UIFontToken.Body_Text)

    // === Collapsed box content: current time + indicator ===
    contentItem: Item {
        UTText {
            id: valueText
            anchors.left: parent.left
            anchors.leftMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            text: control.timeValue
            color: UTComponentUtil.getPlainUIColor(
                       control.textColorEnum,
                       control.enabled ? UIColorState.Normal : UIColorState.Disabled)
            verticalAlignment: Text.AlignVCenter
        }

        UTText {
            anchors.right: parent.right
            anchors.rightMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            text: control.popupIsOpen ? "\u25B4" : "\u25BE"
            color: valueText.color
        }
    }

    background: Rectangle {
        radius: control.borderRadius
        color: UTComponentUtil.getPlainUIColor(
                   control.backgroundColorEnum,
                   control.enabled ? UIColorState.Normal : UIColorState.Disabled)
        border.width: 1
        border.color: UTComponentUtil.getPlainUIColor(
                          control.borderColorEnum,
                          control.activeFocus ? UIColorState.Focused : UIColorState.Normal)
    }

    TapHandler {
        onTapped: control.togglePopup()
    }

    // === Reusable wheel column component ===
    component WheelColumn: Tumbler {
        id: wheel
        property int valueCount: 60
        width: control.autoColumnWidth
        height: control.wheelHeight
        visibleItemCount: control.visibleRows
        wrap: true
        model: valueCount

        // Mouse wheel scrolling (Tumbler does not handle this by default).
        WheelHandler {
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            onWheel: function(event) {
                var step = event.angleDelta.y > 0 ? -1 : 1
                wheel.currentIndex = (wheel.currentIndex + step + wheel.count) % wheel.count
            }
        }

        delegate: Component {
            UTText {
                required property int index
                required property var modelData
                text: (modelData < 10 ? "0" : "") + modelData
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: UTComponentUtil.getPlainUIColor(control.textColorEnum, UIColorState.Normal)
                // Centered row is crisp; neighbours fade with distance.
                opacity: 1.0 - Math.min(0.6, Math.abs(Tumbler.displacement) * 0.28)
                font.pixelSize: Tumbler.displacement === 0 ? 18 : 14
                font.bold: Tumbler.displacement === 0
            }
        }
    }

    // === Drop-down wheels ===
    popup.width: control.width
    popup.padding: 8
    popup.background: Rectangle {
        radius: control.borderRadius
        color: UTComponentUtil.getPlainUIColor(control.backgroundColorEnum, UIColorState.Normal)
        border.width: 1
        border.color: UTComponentUtil.getPlainUIColor(control.borderColorEnum, UIColorState.Normal)
    }

    popup.contentItem: Item {
        implicitHeight: control.wheelHeight

        // Highlight band marking the centered selection row (behind the wheels).
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            height: control.wheelHeight / control.visibleRows
            radius: control.borderRadius - 2
            color: UTComponentUtil.getPlainUIColor(control.columnColorEnum, UIColorState.Selected)
        }

        Row {
            id: wheelRow
            anchors.centerIn: parent
            spacing: control.wheelSpacing

            WheelColumn {
                valueCount: 24
                currentIndex: control.hours
                onCurrentIndexChanged: control.setHours(currentIndex)
            }

            UTText {
                width: control.colonWidth
                text: ":"
                horizontalAlignment: Text.AlignHCenter
                anchors.verticalCenter: parent.verticalCenter
                color: UTComponentUtil.getPlainUIColor(control.textColorEnum, UIColorState.Normal)
            }

            WheelColumn {
                valueCount: 60
                currentIndex: control.minutes
                onCurrentIndexChanged: control.setMinutes(currentIndex)
            }

            UTText {
                visible: control.showSeconds
                width: control.colonWidth
                text: ":"
                horizontalAlignment: Text.AlignHCenter
                anchors.verticalCenter: parent.verticalCenter
                color: UTComponentUtil.getPlainUIColor(control.textColorEnum, UIColorState.Normal)
            }

            WheelColumn {
                visible: control.showSeconds
                valueCount: 60
                currentIndex: control.seconds
                onCurrentIndexChanged: control.setSeconds(currentIndex)
            }
        }
    }

    UTFocusItem {
        target: control.popupIsOpen ? null : control
        focusRadius: control.borderRadius
    }
}
