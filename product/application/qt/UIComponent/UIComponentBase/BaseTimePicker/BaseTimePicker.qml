import QtQuick
import QtQuick.Controls.Basic

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
    property Popup popup: Popup {
        y: control.height
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        onOpened: control.popupIsOpen = true
        onClosed: control.popupIsOpen = false
    }

    // === Keyboard support ===
    Keys.onUpPressed:    function(e) { e.accepted = true; setMinutes(minutes + minuteStep) }
    Keys.onDownPressed:  function(e) { e.accepted = true; setMinutes(minutes - minuteStep) }
    Keys.onSpacePressed: function(e) { e.accepted = true; togglePopup() }
    Keys.onReturnPressed:function(e) { e.accepted = true; togglePopup() }
    Keys.onEscapePressed:function(e) {
        if (popup.visible) { e.accepted = true; popup.close() }
    }
}
