import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Window
import UIComponentBase 1.0

// Time state, wheel composition, rendering and interaction without resource tokens.
Control {
    id: control

    // === Time state ===
    property int hours: 0     // 0-23 (always stored in 24h form)
    property int minutes: 0   // 0-59
    property int seconds: 0   // 0-59

    // === Configuration ===
    property bool showSeconds: false
    property bool use24Hour: true
    property int  minuteStep: 1
    property int  borderRadius: 6

    property color backgroundColor: palette.base
    property color disabledBackgroundColor: backgroundColor
    property color textColor: palette.text
    property color disabledTextColor: textColor
    property color borderColor: palette.mid
    property color focusedBorderColor: palette.highlight
    property color selectedBandColor: palette.highlight
    property font wheelCenteredFont: font
    property font wheelOffCenterFont: font
    property real wheelDisabledOpacity: 1.0
    property real wheelMinimumOpacity: 1.0
    property real wheelDisplacementOpacity: 0.0
    // Compatibility size overrides; UT supplies the design defaults.
    property int wheelCenteredPixelSize: 0
    property int wheelOffCenterPixelSize: 0

    // Read-only structural parts for decoration; retain their value/model bindings.
    readonly property BaseWheelColumn hourColumn: hourCol
    readonly property BaseWheelColumn minuteColumn: minCol
    readonly property BaseWheelColumn secondColumn: secCol

    // === Popup state ===
    property bool popupIsOpen: false

    focusPolicy: Qt.StrongFocus

    // === Read-only formatted output, convenient for bindings ===
    readonly property string timeValue: {
        const pad = n => (n < 10 ? "0" + n : "" + n)
        let h = hours
        let suffix = ""
        if (!use24Hour) {
            suffix = h >= 12 ? " PM" : " AM"
            h = (h % 12) === 0 ? 12 : (h % 12)
        }
        let s = pad(h) + ":" + pad(minutes)
        if (showSeconds)
            s += ":" + pad(seconds)
        return s + suffix
    }

    signal timeChanged()

    // === Normalized setters (wrap-around) ===
    function setHours(v) {
        var nv = ((v % 24) + 24) % 24
        if (nv !== hours) { hours = nv; timeChanged() }
    }
    function setMinutes(v) {
        var nv = ((v % 60) + 60) % 60
        if (nv !== minutes) { minutes = nv; timeChanged() }
    }
    function setSeconds(v) {
        var nv = ((v % 60) + 60) % 60
        if (nv !== seconds) { seconds = nv; timeChanged() }
    }

    function togglePopup() {
        if (popup.visible)
            popup.close()
        else
            popup.open()
    }

    // === Drop-down container ===
    function updatePopupPosition() {
        const ph = popup.height > 0 ? popup.height : popup.implicitHeight
        const scenePos = control.mapToItem(null, 0, 0)
        const winHeight = control.Window.height
        const spaceBelow = winHeight - (scenePos.y + control.height)
        popup.y = (spaceBelow < ph && scenePos.y >= ph) ? -ph : control.height
    }

    property Popup popup: Popup {
        id: dropdown
        parent: control
        // Modal (without the dim) so a press outside that closes the popup is
        // consumed by the overlay instead of passing through to whatever sits
        // below (e.g. an adjacent date picker would otherwise open its popup).
        modal: true
        dim: false
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        onAboutToShow: control.updatePopupPosition()
        onHeightChanged: if (visible) control.updatePopupPosition()
        onOpened: control.popupIsOpen = true
        onClosed: control.popupIsOpen = false

        Connections {
            target: control.Window.window
            function onHeightChanged() { if (dropdown.visible) control.updatePopupPosition() }
        }

        // === Drop-down wheels ===
        width: Math.max(control.width,
            control.minimumColumnWidth * control.wheelCount + control.colonWidth * control.colonCount
                + control.wheelSpacing * (control.wheelCount + control.colonCount - 1)
                + leftPadding + rightPadding)
        padding: 8
        // Let the popup hold keyboard focus so its wheel columns receive keys.
        focus: true
        background: Rectangle {
            radius: control.borderRadius
            color: control.backgroundColor
            border.width: 1
            border.color: control.borderColor
        }

        contentItem: FocusScope {
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
                color: control.selectedBandColor
            }

            Row {
                id: wheelRow
                anchors.centerIn: parent
                spacing: control.wheelSpacing

                BaseWheelColumn {
                    id: hourCol
                    width: control.autoColumnWidth
                    height: control.wheelHeight
                    visibleRows: control.visibleRows
                    wrap: true
                    font: control.font
                    textColor: control.textColor
                    disabledTextColor: control.disabledTextColor
                    centeredPixelSize: control.wheelCenteredPixelSize
                    offCenterPixelSize: control.wheelOffCenterPixelSize
                    centeredFont: control.wheelCenteredFont
                    offCenterFont: control.wheelOffCenterFont
                    disabledOpacity: control.wheelDisabledOpacity
                    minimumOpacity: control.wheelMinimumOpacity
                    displacementOpacity: control.wheelDisplacementOpacity
                    valueCount: 24
                    value: control.hours
                    onValuePicked: function(v) { control.setHours(v) }
                }

                BaseText {
                    font: control.font
                    width: control.colonWidth
                    text: ":"
                    horizontalAlignment: Text.AlignHCenter
                    anchors.verticalCenter: parent.verticalCenter
                    color: control.textColor
                }

                BaseWheelColumn {
                    id: minCol
                    width: control.autoColumnWidth
                    height: control.wheelHeight
                    visibleRows: control.visibleRows
                    wrap: true
                    font: control.font
                    textColor: control.textColor
                    disabledTextColor: control.disabledTextColor
                    centeredPixelSize: control.wheelCenteredPixelSize
                    offCenterPixelSize: control.wheelOffCenterPixelSize
                    centeredFont: control.wheelCenteredFont
                    offCenterFont: control.wheelOffCenterFont
                    disabledOpacity: control.wheelDisabledOpacity
                    minimumOpacity: control.wheelMinimumOpacity
                    displacementOpacity: control.wheelDisplacementOpacity
                    valueCount: 60
                    value: control.minutes
                    onValuePicked: function(v) { control.setMinutes(v) }
                }

                BaseText {
                    font: control.font
                    visible: control.showSeconds
                    width: control.colonWidth
                    text: ":"
                    horizontalAlignment: Text.AlignHCenter
                    anchors.verticalCenter: parent.verticalCenter
                    color: control.textColor
                }

                BaseWheelColumn {
                    id: secCol
                    visible: control.showSeconds
                    width: control.autoColumnWidth
                    height: control.wheelHeight
                    visibleRows: control.visibleRows
                    wrap: true
                    font: control.font
                    textColor: control.textColor
                    disabledTextColor: control.disabledTextColor
                    centeredPixelSize: control.wheelCenteredPixelSize
                    offCenterPixelSize: control.wheelOffCenterPixelSize
                    centeredFont: control.wheelCenteredFont
                    offCenterFont: control.wheelOffCenterFont
                    disabledOpacity: control.wheelDisabledOpacity
                    minimumOpacity: control.wheelMinimumOpacity
                    displacementOpacity: control.wheelDisplacementOpacity
                    valueCount: 60
                    value: control.seconds
                    onValuePicked: function(v) { control.setSeconds(v) }
                }
            }
        }
    }

    // === Keyboard support ===
    // Closed: Up/Down nudges the minutes, Space/Enter opens the wheels. While
    // open, focus lives in the popup; the wheel columns handle Up/Down and the
    // popup focus scope handles Left/Right (column focus) and confirm/close.
    Keys.onUpPressed: function(e) {
        if (!popupIsOpen) { e.accepted = true; setMinutes(minutes + minuteStep) }
    }
    Keys.onDownPressed: function(e) {
        if (!popupIsOpen) { e.accepted = true; setMinutes(minutes - minuteStep) }
    }
    Keys.onSpacePressed: function(e) { e.accepted = true; togglePopup() }
    Keys.onReturnPressed:function(e) { e.accepted = true; togglePopup() }
    Keys.onEnterPressed: function(e) { e.accepted = true; togglePopup() }
    Keys.onEscapePressed:function(e) {
        if (popup.visible) { e.accepted = true; popup.close() }
    }
    // When this box receives Tab/Backtab, close its popup and allow focus navigation.
    // An open popup handles wheel focus through its own focus chain.
    // Named key signals accept by default; clear accepted to continue navigation.
    Keys.onTabPressed:    function(e) { if (popup.visible) popup.close(); e.accepted = false }
    Keys.onBacktabPressed:function(e) { if (popup.visible) popup.close(); e.accepted = false }

    // === Wheel geometry ===
    property int columnWidth: 56
    property int wheelHeight: 160
    property int visibleRows: 5
    property int wheelSpacing: 1
    property int colonWidth: 10
    property int minimumColumnWidth: 36

    // Number of wheel columns / colon separators currently shown.
    readonly property int wheelCount: showSeconds ? 3 : 2
    readonly property int colonCount: wheelCount - 1
    // Column width that makes the wheels fill the popup width evenly.
    readonly property real autoColumnWidth: {
        var inner = popup.availableWidth
        var used = colonCount * colonWidth + wheelSpacing * (wheelCount + colonCount - 1)
        return Math.max(minimumColumnWidth, (inner - used) / wheelCount)
    }

    implicitWidth: 180
    implicitHeight: 32
    padding: 0

    // === Collapsed box content: current time + indicator ===
    contentItem: Item {
        BaseText {
            font: control.font
            id: valueText
            anchors.left: parent.left
            anchors.leftMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            text: control.timeValue
            color: (control.enabled ? control.textColor : control.disabledTextColor)
            verticalAlignment: Text.AlignVCenter
        }

        BaseText {
            font: control.font
            anchors.right: parent.right
            anchors.rightMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            text: control.popupIsOpen ? "\u25B4" : "\u25BE"
            color: valueText.color
        }
    }

    background: Rectangle {
        radius: control.borderRadius
        color: (control.enabled ? control.backgroundColor : control.disabledBackgroundColor)
        border.width: 1
        border.color: (control.activeFocus ? control.focusedBorderColor : control.borderColor)
    }

    // MouseArea (not TapHandler) so another picker's modal popup overlay blocks
    // the click - a TapHandler would still fire through the overlay.
    MouseArea {
        anchors.fill: parent
        onClicked: control.togglePopup()
    }
}
