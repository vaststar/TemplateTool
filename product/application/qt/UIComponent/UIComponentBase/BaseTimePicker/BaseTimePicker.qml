import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Window

/**
 * BaseTimePicker - Unstyled time picker behavior.
 *
 * Holds the time state (hours/minutes/seconds), normalization with wrap-around,
 * 12/24-hour formatting and an (unstyled) drop-down Popup that the UT layer
 * fills with scrolling wheels (Tumbler columns). No colors / fonts here; the
 * UTComponent layer applies design tokens, matching the Base + UT pattern used
 * by BaseComboBox / UTComboBox.
 */
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

    // === Popup state (for subclass styling) ===
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

    // === Drop-down container (styling provided by UT layer) ===
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
    }

    // === Keyboard support ===
    // Closed: Up/Down nudges the minutes, Space/Enter opens the wheels. While
    // open, focus lives in the popup; the wheel columns handle Up/Down and the
    // UT layer handles Left/Right (column focus) and confirm/close.
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
    // Tab/Backtab moves focus out of the picker; close the wheels first but let
    // the default focus navigation proceed. Named key signals default
    // e.accepted to true, so we must clear it explicitly or focus won't move.
    Keys.onTabPressed:    function(e) { if (popup.visible) popup.close(); e.accepted = false }
    Keys.onBacktabPressed:function(e) { if (popup.visible) popup.close(); e.accepted = false }
}
