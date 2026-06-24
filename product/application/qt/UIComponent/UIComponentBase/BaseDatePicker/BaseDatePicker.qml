import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Window

/**
 * BaseDatePicker - Unstyled date picker state core.
 *
 * Holds only the selected date, optional [minDate, maxDate] range and the
 * (unstyled) drop-down Popup container. No presentation (formatting), no
 * calendar-browsing notions (browsed month, page navigation, view modes) and
 * no colors / fonts - those belong to the UT layer that renders a concrete
 * style (calendar grid in UTDatePicker, scrolling wheels in UTDateWheelPicker).
 *
 * Shared by every date-picker style; each UT subclass fills popup.contentItem
 * with its own editor and reads back through selectDate().
 */
Control {
    id: control

    // === State ===
    property date selectedDate: new Date()
    property var minDate: undefined   // Date or undefined
    property var maxDate: undefined   // Date or undefined

    // === Configuration ===
    property int borderRadius: 6

    // === Popup state (for subclass styling) ===
    property bool popupIsOpen: false

    focusPolicy: Qt.StrongFocus

    signal dateChanged()
    // Emitted just before the popup shows, so a style can sync its editor to
    // selectedDate (e.g. the calendar resets its browsed month / view).
    signal popupAboutToShow()

    // === Core date logic ===
    function isInRange(d) {
        if (minDate !== undefined && d < minDate)
            return false
        if (maxDate !== undefined && d > maxDate)
            return false
        return true
    }

    // Whether the inclusive interval [from, to] overlaps [minDate, maxDate].
    // Use this to decide if a whole month/year contains any selectable day,
    // instead of probing a few sample dates (which misses partially-valid
    // months/years at a boundary).
    function rangeOverlaps(from, to) {
        if (maxDate !== undefined && from > maxDate)
            return false
        if (minDate !== undefined && to < minDate)
            return false
        return true
    }

    function isSameDay(a, b) {
        return a.getFullYear() === b.getFullYear()
            && a.getMonth() === b.getMonth()
            && a.getDate() === b.getDate()
    }

    function selectDate(d) {
        if (!isInRange(d))
            return
        selectedDate = d
        dateChanged()
    }

    // === Neutral date math (useful to any style) ===
    // Number of days in the given (year, 0-based month).
    function daysInMonth(year, month) {
        return new Date(year, month + 1, 0).getDate()
    }

    // Clamp a day-of-month to the last valid day of (year, month).
    function clampDay(year, month, day) {
        return Math.min(day, daysInMonth(year, month))
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
        // below (e.g. an adjacent time picker would otherwise open its popup).
        modal: true
        dim: false
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        onAboutToShow: {
            control.popupAboutToShow()
            control.updatePopupPosition()
        }
        onHeightChanged: if (visible) control.updatePopupPosition()
        onOpened: control.popupIsOpen = true
        onClosed: control.popupIsOpen = false

        Connections {
            target: control.Window.window
            function onHeightChanged() { if (dropdown.visible) control.updatePopupPosition() }
        }
    }

    // === Keyboard support (popup open/close only; styles add navigation) ===
    Keys.onSpacePressed:  function(e) { e.accepted = true; togglePopup() }
    Keys.onReturnPressed: function(e) { e.accepted = true; togglePopup() }
    Keys.onEscapePressed: function(e) {
        if (popup.visible) { e.accepted = true; popup.close() }
    }
}
