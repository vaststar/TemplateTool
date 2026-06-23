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
        // Position of this column (0=hours, 1=minutes, 2=seconds) used to sync
        // with the control's keyboard-driven active column.
        property int columnIndex: 0
        // Source-of-truth value coming from the control. Kept separate from
        // currentIndex so external (keyboard) changes can reposition the wheel
        // without fighting a two-way binding.
        property int value: 0
        signal valuePicked(int v)
        readonly property bool active: control.popupIsOpen && control.activeColumn === columnIndex
        width: control.autoColumnWidth
        height: control.wheelHeight
        visibleItemCount: control.visibleRows
        wrap: true
        model: valueCount

        // External value -> wheel: imperatively move the Tumbler (guarded so the
        // resulting onCurrentIndexChanged does not loop back).
        onValueChanged: if (currentIndex !== value) currentIndex = value
        Component.onCompleted: currentIndex = value
        // Wheel -> control: report user-driven changes only.
        onCurrentIndexChanged: if (currentIndex !== value) valuePicked(currentIndex)

        // Blue focus outline for the keyboard-driven column. Driven by the same
        // KeyboardFocusTracker as UTFocusItem, so it only shows during keyboard
        // navigation and stays hidden for mouse interaction.
        UTFocusItem {
            target: wheel
            delegateFocused: wheel.active && control.activeFocus
            focusRadius: control.borderRadius - 2
        }

        // Clicking a column makes it the keyboard-active one.
        TapHandler {
            onTapped: control.activeColumn = wheel.columnIndex
        }

        // Mouse wheel scrolling (Tumbler does not handle this by default).
        WheelHandler {
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            onWheel: function(event) {
                control.activeColumn = wheel.columnIndex
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
                columnIndex: 0
                value: control.hours
                onValuePicked: function(v) { control.setHours(v) }
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
                columnIndex: 1
                value: control.minutes
                onValuePicked: function(v) { control.setMinutes(v) }
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
                columnIndex: 2
                value: control.seconds
                onValuePicked: function(v) { control.setSeconds(v) }
            }
        }
    }

    UTFocusItem {
        target: control.popupIsOpen ? null : control
        focusRadius: control.borderRadius
    }
}
