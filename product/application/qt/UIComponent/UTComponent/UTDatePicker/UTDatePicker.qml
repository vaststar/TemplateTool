import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic
import QtQuick.Layouts
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTDatePicker - Themed date picker with a calendar grid (MonthGrid).
 *
 * Collapsed: an input-style box showing the selected date (e.g. "2026-06-17").
 * Expanded: a drop-down calendar with month navigation. Days outside the
 * browsed month fade out; the selected day gets an accent background; days
 * outside [minDate, maxDate] are disabled. Follows the Base + UT token pattern
 * used by UTTimePicker.
 *
 * Usage:
 *   UTDatePicker {
 *       selectedDate: new Date()
 *       onDateChanged: myModel.date = dateValue
 *   }
 */
BaseDatePicker {
    id: control

    // === Theme tokens ===
    property var backgroundColorEnum: UIColorToken.Datepicker_Background
    property var textColorEnum:       UIColorToken.Datepicker_Text
    property var borderColorEnum:     UIColorToken.Datepicker_Border
    property var todayColorEnum:      UIColorToken.Datepicker_Today_Background
    property var selectedColorEnum:   UIColorToken.Datepicker_Selected_Background

    property var calendarLocale: Qt.locale(Qt.uiLanguage)

    // === Presentation (collapsed box text) ===
    property string displayFormat: "yyyy-MM-dd"
    readonly property string dateValue: Qt.formatDate(selectedDate, displayFormat)

    // === Calendar browsing state (this style only) ===
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

    // Keyboard browsing is separate from the committed selection.
    property date focusedDate: selectedDate
    property int focusedMonth: displayMonth
    property int focusedYear: displayYear
    readonly property Item activeGrid: viewMode === daysView ? grid
        : viewMode === monthsView ? monthGrid : yearGrid
    readonly property int focusedCell: viewMode === daysView ? focusedDate.getDate()
        : viewMode === monthsView ? focusedMonth : focusedYear
    readonly property bool hasFocusedCell: cellAllowed(focusedCell)
        && (viewMode !== daysView || (focusedDate.getFullYear() === displayYear
                                     && focusedDate.getMonth() === displayMonth))
        && (viewMode !== yearsView || (focusedYear >= yearBlockStart
                                      && focusedYear < yearBlockStart + 12))

    // Keep the browsed month in sync when selectedDate is set externally.
    onSelectedDateChanged: {
        displayYear = selectedDate.getFullYear()
        displayMonth = selectedDate.getMonth()
        focusedDate = new Date(displayYear, displayMonth, selectedDate.getDate())
        focusedMonth = displayMonth
        focusedYear = displayYear
        syncGridCursor()
    }

    // Reset the browsed month/view to the selected date when the popup opens.
    onPopupAboutToShow: resetView()

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
        syncGridCursor()
    }

    function navigateNext() {
        if (viewMode === daysView)
            nextMonth()
        else if (viewMode === monthsView)
            displayYear++
        else
            displayYear += 12
        syncGridCursor()
    }

    // === View switching ===
    function showMonths() {
        focusedMonth = displayMonth
        viewMode = monthsView
        focusCalendar()
    }
    function showYears() {
        focusedYear = displayYear
        viewMode = yearsView
        focusCalendar()
    }

    function pickMonth(m) {
        displayMonth = m
        viewMode = daysView
        focusCalendar()
    }

    function pickYear(y) {
        displayYear = y
        viewMode = monthsView
        focusCalendar()
    }

    function resetView() {
        displayYear = selectedDate.getFullYear()
        displayMonth = selectedDate.getMonth()
        viewMode = daysView
        focusedDate = new Date(displayYear, displayMonth, selectedDate.getDate())
        focusedMonth = displayMonth
        focusedYear = displayYear
        syncGridCursor()
    }

    function cellAllowed(value) {
        if (viewMode === daysView)
            return isInRange(new Date(displayYear, displayMonth, value))
        const y = viewMode === monthsView ? displayYear : value
        const m = viewMode === monthsView ? value : 0
        const lastMonth = viewMode === monthsView ? value : 11
        return rangeOverlaps(new Date(y, m, 1),
            new Date(y, lastMonth + 1, 0, 23, 59, 59, 999))
    }

    // Pick the nearest enabled cell on the displayed page, including partial ranges.
    function syncGridCursor() {
        const first = viewMode === daysView ? 1 : viewMode === monthsView ? 0 : yearBlockStart
        const count = viewMode === daysView ? daysInMonth(displayYear, displayMonth) : 12
        let nearest = undefined
        for (let value = first; value < first + count; ++value) {
            if (cellAllowed(value) && (nearest === undefined
                    || Math.abs(value - focusedCell) < Math.abs(nearest - focusedCell)))
                nearest = value
        }
        if (nearest === undefined)
            return false
        if (viewMode === daysView)
            focusedDate = new Date(displayYear, displayMonth, nearest)
        else if (viewMode === monthsView)
            focusedMonth = nearest
        else
            focusedYear = nearest
        return true
    }

    function focusCalendar() {
        if (popup.visible) {
            const target = syncGridCursor() ? activeGrid : titleButton
            target.forceActiveFocus(Qt.OtherFocusReason)
        }
    }

    function moveGridFocus(delta) {
        if (!hasFocusedCell)
            return
        if (viewMode === daysView) {
            const next = new Date(displayYear, displayMonth, focusedDate.getDate() + delta)
            if (!isInRange(next))
                return
            if (next.getFullYear() !== displayYear || next.getMonth() !== displayMonth) {
                if (delta < 0) navigatePrev()
                else navigateNext()
            }
            focusedDate = next
        } else if (viewMode === monthsView) {
            const next = new Date(displayYear, focusedMonth + delta, 1)
            if (!rangeOverlaps(next,
                    new Date(next.getFullYear(), next.getMonth() + 1, 0, 23, 59, 59, 999)))
                return
            if (next.getFullYear() < displayYear) navigatePrev()
            else if (next.getFullYear() > displayYear) navigateNext()
            focusedMonth = next.getMonth()
        } else {
            const next = focusedYear + delta
            if (!cellAllowed(next))
                return
            if (next < yearBlockStart) navigatePrev()
            else if (next >= yearBlockStart + 12) navigateNext()
            focusedYear = next
        }
    }

    function confirmGridFocus() {
        if (!hasFocusedCell)
            return
        if (viewMode === daysView) {
            selectDate(focusedDate)
            popup.close()
        } else if (viewMode === monthsView) {
            pickMonth(focusedMonth)
        } else {
            pickYear(focusedYear)
        }
    }

    function handleGridKey(event) {
        const rowStep = viewMode === daysView ? 7 : 3
        switch (event.key) {
        case Qt.Key_Left:  moveGridFocus(-1);       break
        case Qt.Key_Right: moveGridFocus(1);        break
        case Qt.Key_Up:    moveGridFocus(-rowStep); break
        case Qt.Key_Down:  moveGridFocus(rowStep);  break
        case Qt.Key_PageUp:   navigatePrev(); focusCalendar(); break
        case Qt.Key_PageDown: navigateNext(); focusCalendar(); break
        case Qt.Key_Return:
        case Qt.Key_Enter:
        case Qt.Key_Space: confirmGridFocus(); break
        default: event.accepted = false; return
        }
        event.accepted = true
    }

    // === Geometry ===
    property int cellSize: 34
    property int popupPadding: 10
    property int popupWidth: cellSize * 7 + popupPadding * 2

    implicitWidth: 180
    implicitHeight: 32
    padding: 0

    font: UTComponentUtil.getUIFont(UIFontToken.Body_Text)

    // === Collapsed box content: current date + indicator ===
    contentItem: Item {
        UTText {
            id: valueText
            anchors.left: parent.left
            anchors.leftMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            text: control.dateValue
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

    // === Drop-down calendar ===
    Keys.onEnterPressed: function(event) { event.accepted = true; togglePopup() }
    popup.focus: true
    popup.width: control.popupWidth
    popup.padding: control.popupPadding
    popup.background: Rectangle {
        radius: control.borderRadius
        color: UTComponentUtil.getPlainUIColor(control.backgroundColorEnum, UIColorState.Normal)
        border.width: 1
        border.color: UTComponentUtil.getPlainUIColor(control.borderColorEnum, UIColorState.Normal)
    }

    popup.contentItem: ColumnLayout {
        spacing: 8

        Connections {
            target: control.popup
            function onOpened() { control.focusCalendar() }
            function onClosed() { control.forceActiveFocus(Qt.PopupFocusReason) }
        }

        // ── Header: prev / title / next ──
        RowLayout {
            Layout.fillWidth: true
            spacing: 4

            UTToolButton {
                id: previousButton
                objectName: "calendarPreviousButton"
                focusPolicy: Qt.StrongFocus
                KeyNavigation.tab: titleButton
                KeyNavigation.backtab: control.hasFocusedCell ? control.activeGrid : nextButton
                Keys.onReturnPressed: function(event) { event.accepted = true; previousButton.clicked() }
                Keys.onEnterPressed: function(event) { event.accepted = true; previousButton.clicked() }
                Layout.preferredWidth: control.cellSize
                Layout.preferredHeight: control.cellSize
                text: "\u2039"
                backgroundColorEnum: UIColorToken.Datepicker_Background
                fontColorEnum: control.textColorEnum
                onClicked: control.navigatePrev()
            }

            // Clickable title: days -> months -> years.
            UTToolButton {
                id: titleButton
                objectName: "calendarTitleButton"
                focusPolicy: Qt.StrongFocus
                KeyNavigation.tab: nextButton
                KeyNavigation.backtab: previousButton
                Keys.onReturnPressed: function(event) { event.accepted = true; titleButton.clicked() }
                Keys.onEnterPressed: function(event) { event.accepted = true; titleButton.clicked() }
                Layout.fillWidth: true
                Layout.preferredHeight: control.cellSize
                backgroundColorEnum: UIColorToken.Datepicker_Background
                fontColorEnum: control.textColorEnum
                text: {
                    if (control.viewMode === control.monthsView)
                        return control.displayYear
                    if (control.viewMode === control.yearsView)
                        return control.yearBlockStart + " - " + (control.yearBlockStart + 11)
                    const monthName = control.calendarLocale.standaloneMonthName(control.displayMonth, Locale.LongFormat)
                    const lang = control.calendarLocale.name
                    const yearFirst = lang.startsWith("zh") || lang.startsWith("ja") || lang.startsWith("ko")
                    return yearFirst ? (control.displayYear + " " + monthName)
                                     : (monthName + " " + control.displayYear)
                }
                onClicked: {
                    if (control.viewMode === control.daysView)
                        control.showMonths()
                    else if (control.viewMode === control.monthsView)
                        control.showYears()
                }
            }

            UTToolButton {
                id: nextButton
                objectName: "calendarNextButton"
                focusPolicy: Qt.StrongFocus
                KeyNavigation.tab: control.hasFocusedCell ? control.activeGrid : previousButton
                KeyNavigation.backtab: titleButton
                Keys.onReturnPressed: function(event) { event.accepted = true; nextButton.clicked() }
                Keys.onEnterPressed: function(event) { event.accepted = true; nextButton.clicked() }
                Layout.preferredWidth: control.cellSize
                Layout.preferredHeight: control.cellSize
                text: "\u203A"
                backgroundColorEnum: UIColorToken.Datepicker_Background
                fontColorEnum: control.textColorEnum
                onClicked: control.navigateNext()
            }
        }

        // ── Day view ──
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8
            visible: control.viewMode === control.daysView

            DayOfWeekRow {
                Layout.fillWidth: true
                spacing: 0
                locale: control.calendarLocale

                delegate: Item {
                    required property var model
                    implicitWidth: control.cellSize
                    implicitHeight: control.cellSize

                    UTText {
                        anchors.centerIn: parent
                        width: parent.width
                        text: model.shortName
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideNone
                        color: UTComponentUtil.getPlainUIColor(control.textColorEnum, UIColorState.Normal)
                        opacity: 0.7
                    }
                }
            }

            MonthGrid {
                id: grid
                objectName: "calendarDayGrid"
                activeFocusOnTab: true
                KeyNavigation.tab: previousButton
                KeyNavigation.backtab: nextButton
                Keys.onPressed: function(event) { control.handleGridKey(event) }
                Layout.fillWidth: true
                spacing: 0
                month: control.displayMonth
                year: control.displayYear
                locale: control.calendarLocale

                delegate: Item {
                    required property var model
                    implicitWidth: control.cellSize
                    implicitHeight: control.cellSize

                    readonly property bool inThisMonth: model.month === control.displayMonth
                    readonly property bool isSelected: control.isSameDay(model.date, control.selectedDate)
                    readonly property bool isToday: control.isSameDay(model.date, new Date())
                    readonly property bool allowed: control.isInRange(model.date)

                    HoverHandler {
                        id: dayHover
                        enabled: parent.allowed
                    }

                    Rectangle {
                        anchors.centerIn: parent
                        width: control.cellSize - 4
                        height: control.cellSize - 4
                        radius: (control.cellSize - 4) / 2
                        color: parent.isSelected
                            ? UTComponentUtil.getPlainUIColor(control.selectedColorEnum, UIColorState.Selected)
                            : dayHover.hovered
                                ? UTComponentUtil.getPlainUIColor(control.todayColorEnum, UIColorState.Hovered)
                            : parent.isToday
                                ? UTComponentUtil.getPlainUIColor(control.todayColorEnum, UIColorState.Normal)
                                : "transparent"

                        UTText {
                            anchors.centerIn: parent
                            text: model.day
                            color: UTComponentUtil.getPlainUIColor(control.textColorEnum, UIColorState.Normal)
                            opacity: !parent.parent.allowed ? 0.3
                                   : parent.parent.inThisMonth ? 1.0 : 0.35
                        }

                        UTFocusItem {
                            focusRadius: height / 2
                            focusMargin: 0
                            delegateFocused: grid.activeFocus && parent.parent.allowed
                                && control.isSameDay(model.date, control.focusedDate)
                        }
                    }

                    TapHandler {
                        enabled: parent.allowed
                        onTapped: {
                            control.selectDate(model.date)
                            control.popup.close()
                        }
                    }
                }
            }
        }

        // ── Month view ──
        Grid {
            id: monthGrid
            objectName: "calendarMonthGrid"
            activeFocusOnTab: true
            KeyNavigation.tab: previousButton
            KeyNavigation.backtab: nextButton
            Keys.onPressed: function(event) { control.handleGridKey(event) }
            Layout.fillWidth: true
            visible: control.viewMode === control.monthsView
            columns: 3
            spacing: 6

            Repeater {
                model: 12
                delegate: Item {
                    required property int index
                    width: (control.popupWidth - control.popupPadding * 2 - 12) / 3
                    height: control.cellSize + 10

                    readonly property bool isSelected: index === control.displayMonth
                        && control.displayYear === control.selectedDate.getFullYear()
                    // Allowed if any day of the month falls within range.
                    readonly property bool allowed: control.rangeOverlaps(
                        new Date(control.displayYear, index, 1),
                        new Date(control.displayYear, index + 1, 0, 23, 59, 59, 999))

                    HoverHandler {
                        id: monthHover
                        enabled: parent.allowed
                    }

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 2
                        radius: control.borderRadius - 2
                        color: parent.isSelected
                            ? UTComponentUtil.getPlainUIColor(control.selectedColorEnum, UIColorState.Selected)
                            : monthHover.hovered
                                ? UTComponentUtil.getPlainUIColor(control.todayColorEnum, UIColorState.Hovered)
                            : "transparent"

                        UTText {
                            anchors.centerIn: parent
                            text: control.calendarLocale.standaloneMonthName(index, Locale.ShortFormat)
                            color: UTComponentUtil.getPlainUIColor(control.textColorEnum, UIColorState.Normal)
                            opacity: parent.parent.allowed ? 1.0 : 0.3
                        }

                        UTFocusItem {
                            focusRadius: control.borderRadius - 2
                            focusMargin: 0
                            delegateFocused: monthGrid.activeFocus && parent.parent.allowed
                                && index === control.focusedMonth
                        }
                    }

                    TapHandler {
                        enabled: parent.allowed
                        onTapped: control.pickMonth(index)
                    }
                }
            }
        }

        // ── Year view ──
        Grid {
            id: yearGrid
            objectName: "calendarYearGrid"
            activeFocusOnTab: true
            KeyNavigation.tab: previousButton
            KeyNavigation.backtab: nextButton
            Keys.onPressed: function(event) { control.handleGridKey(event) }
            Layout.fillWidth: true
            visible: control.viewMode === control.yearsView
            columns: 3
            spacing: 6

            Repeater {
                model: 12
                delegate: Item {
                    required property int index
                    readonly property int year: control.yearBlockStart + index
                    width: (control.popupWidth - control.popupPadding * 2 - 12) / 3
                    height: control.cellSize + 10

                    readonly property bool isSelected: year === control.selectedDate.getFullYear()
                    readonly property bool allowed: control.rangeOverlaps(
                        new Date(year, 0, 1),
                        new Date(year, 11, 31, 23, 59, 59, 999))

                    HoverHandler {
                        id: yearHover
                        enabled: parent.allowed
                    }

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 2
                        radius: control.borderRadius - 2
                        color: parent.isSelected
                            ? UTComponentUtil.getPlainUIColor(control.selectedColorEnum, UIColorState.Selected)
                            : yearHover.hovered
                                ? UTComponentUtil.getPlainUIColor(control.todayColorEnum, UIColorState.Hovered)
                            : "transparent"

                        UTText {
                            anchors.centerIn: parent
                            text: parent.parent.year
                            color: UTComponentUtil.getPlainUIColor(control.textColorEnum, UIColorState.Normal)
                            opacity: parent.parent.allowed ? 1.0 : 0.3
                        }

                        UTFocusItem {
                            focusRadius: control.borderRadius - 2
                            focusMargin: 0
                            delegateFocused: yearGrid.activeFocus && parent.parent.allowed
                                && parent.parent.year === control.focusedYear
                        }
                    }

                    TapHandler {
                        enabled: parent.allowed
                        onTapped: control.pickYear(parent.year)
                    }
                }
            }
        }
    }

    UTFocusItem {
        target: control.popupIsOpen ? null : control
        focusRadius: control.borderRadius
    }
}
