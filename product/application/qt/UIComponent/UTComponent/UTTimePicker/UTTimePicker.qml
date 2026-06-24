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

    // MouseArea (not TapHandler) so another picker's modal popup overlay blocks
    // the click - a TapHandler would still fire through the overlay.
    MouseArea {
        anchors.fill: parent
        onClicked: control.togglePopup()
    }

    // === Drop-down wheels ===
    popup.width: control.width
    popup.padding: 8
    // Let the popup hold keyboard focus so its wheel columns receive keys.
    popup.focus: true
    popup.background: Rectangle {
        radius: control.borderRadius
        color: UTComponentUtil.getPlainUIColor(control.backgroundColorEnum, UIColorState.Normal)
        border.width: 1
        border.color: UTComponentUtil.getPlainUIColor(control.borderColorEnum, UIColorState.Normal)
    }

    popup.contentItem: FocusScope {
        implicitHeight: control.wheelHeight

        // Focus the first column when the wheels open; hand focus back to the
        // box when they close.
        Connections {
            target: control.popup
            function onOpened() { hourCol.forceActiveFocus() }
            function onClosed() { control.forceActiveFocus() }
        }

        // Move keyboard focus between columns. Bubbled up from the focused
        // wheel, which leaves Left/Right unhandled on purpose.
        function moveColumn(delta) {
            var cols = [hourCol, minCol]
            if (control.showSeconds) cols.push(secCol)
            var idx = 0
            for (var i = 0; i < cols.length; ++i)
                if (cols[i].activeFocus) { idx = i; break }
            cols[((idx + delta) % cols.length + cols.length) % cols.length].forceActiveFocus()
        }
        Keys.onLeftPressed:  function(e) { moveColumn(-1); e.accepted = true }
        Keys.onRightPressed: function(e) { moveColumn(1);  e.accepted = true }
        Keys.onSpacePressed: function(e) { control.popup.close(); e.accepted = true }
        Keys.onReturnPressed:function(e) { control.popup.close(); e.accepted = true }
        Keys.onEnterPressed: function(e) { control.popup.close(); e.accepted = true }

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

            UTWheelColumn {
                id: hourCol
                width: control.autoColumnWidth
                height: control.wheelHeight
                visibleRows: control.visibleRows
                wrap: true
                textColorEnum: control.textColorEnum
                borderColorEnum: control.borderColorEnum
                borderRadius: control.borderRadius
                valueCount: 24
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

            UTWheelColumn {
                id: minCol
                width: control.autoColumnWidth
                height: control.wheelHeight
                visibleRows: control.visibleRows
                wrap: true
                textColorEnum: control.textColorEnum
                borderColorEnum: control.borderColorEnum
                borderRadius: control.borderRadius
                valueCount: 60
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

            UTWheelColumn {
                id: secCol
                visible: control.showSeconds
                width: control.autoColumnWidth
                height: control.wheelHeight
                visibleRows: control.visibleRows
                wrap: true
                textColorEnum: control.textColorEnum
                borderColorEnum: control.borderColorEnum
                borderRadius: control.borderRadius
                valueCount: 60
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
