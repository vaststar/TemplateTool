import QtQuick
import QtQuick.Controls.Basic

/**
 * BaseDatePicker - Unstyled date picker behavior.
 *
 * Holds the selected date, the currently browsed year/month, month navigation
 * and optional min/max range checks, plus an (unstyled) drop-down Popup that
 * the UT layer fills with a calendar grid. No colors / fonts here; the
 * UTComponent layer applies design tokens, matching the Base + UT pattern used
 * by BaseTimePicker / UTTimePicker.
 */
Control {
    id: control

    // === State ===
    property date selectedDate: new Date()
    property string displayFormat: "yyyy-MM-dd"
    property var minDate: undefined   // Date or undefined
    property var maxDate: undefined   // Date or undefined
    property int firstDayOfWeek: Qt.Sunday

    // Currently browsed month (may differ from selectedDate's month).
    property int displayYear: selectedDate.getFullYear()
    property int displayMonth: selectedDate.getMonth()   // 0-11

    // Calendar view: 0 = days, 1 = months, 2 = years.
    property int viewMode: 0
    readonly property int daysView: 0
    readonly property int monthsView: 1
    readonly property int yearsView: 2

    // First year shown in the 12-year block of the year view.
    readonly property int yearBlockStart: displayYear - (displayYear % 12)

    // === Configuration ===
    property int borderRadius: 6

    // === Popup state (for subclass styling) ===
    property bool popupIsOpen: false

    focusPolicy: Qt.StrongFocus

    // === Read-only formatted output, convenient for bindings ===
    readonly property string dateValue: Qt.formatDate(selectedDate, displayFormat)

    signal dateChanged()

    // Keep the browsed month in sync when selectedDate is set externally.
    onSelectedDateChanged: {
        displayYear = selectedDate.getFullYear()
        displayMonth = selectedDate.getMonth()
    }

    function isInRange(d) {
        if (minDate !== undefined && d < minDate)
            return false
        if (maxDate !== undefined && d > maxDate)
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

    // === Month navigation (day view) ===
    function prevMonth() {
        if (displayMonth === 0) {
            displayMonth = 11
            displayYear--
        } else {
            displayMonth--
        }
    }

    function nextMonth() {
        if (displayMonth === 11) {
            displayMonth = 0
            displayYear++
        } else {
            displayMonth++
        }
    }

    // === Context-aware prev/next for the arrow buttons ===
    function navigatePrev() {
        if (viewMode === daysView)
            prevMonth()
        else if (viewMode === monthsView)
            displayYear--
        else
            displayYear -= 12
    }

    function navigateNext() {
        if (viewMode === daysView)
            nextMonth()
        else if (viewMode === monthsView)
            displayYear++
        else
            displayYear += 12
    }

    // === View switching ===
    function showMonths() { viewMode = monthsView }
    function showYears()  { viewMode = yearsView }

    function pickMonth(m) {
        displayMonth = m
        viewMode = daysView
    }

    function pickYear(y) {
        displayYear = y
        viewMode = monthsView
    }

    // Reset the browsed month/view to the selected date.
    function resetView() {
        displayYear = selectedDate.getFullYear()
        displayMonth = selectedDate.getMonth()
        viewMode = daysView
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
        onAboutToShow: control.resetView()
        onOpened: control.popupIsOpen = true
        onClosed: control.popupIsOpen = false
    }

    // === Keyboard support ===
    Keys.onSpacePressed:  function(e) { e.accepted = true; togglePopup() }
    Keys.onReturnPressed: function(e) { e.accepted = true; togglePopup() }
    Keys.onEscapePressed: function(e) {
        if (popup.visible) { e.accepted = true; popup.close() }
    }
}
