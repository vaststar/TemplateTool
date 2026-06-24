import QtQuick
import QtQuick.Layouts
import UIComponentBase 1.0
import UTComponent 1.0
import UTComposite 1.0
import UIResourceLoader 1.0

/**
 * UTDateTimeRangePicker - Composite range picker built from a date-picker +
 * UTTimePicker pair on each side (start / end).
 *
 * Two modes:
 *   - Date     : pick a date interval (time pickers hidden).
 *   - DateTime : pick a date + time interval (default).
 *
 * Two date-picker styles:
 *   - Calendar : UTDatePicker (grid, default).
 *   - Wheel    : UTDateWheelPicker (scrolling year/month/day wheels).
 *
 * Ordering is auto-clamped: the range is always kept valid (start <= end). When
 * one side is moved past the other, the edited side is clamped to the other (the
 * opposite side stays put), so `startDate` is never later than `endDate`. The
 * calendars also cross-limit each other so illegal days are visually disabled.
 *
 * Usage:
 *   UTDateTimeRangePicker {
 *       mode: UTDateTimeRangePicker.DateTime
 *       datePickerStyle: UTDateTimeRangePicker.Wheel
 *       startDate: new Date()
 *       endDate:   new Date()
 *       onRangeChanged: (start, end) => myModel.setRange(start, end)
 *   }
 */
Item {
    id: control

    enum Mode { Date, DateTime }
    enum DateStyle { Calendar, Wheel }

    // === Configuration ===
    property int  mode: UTDateTimeRangePicker.DateTime
    property int  datePickerStyle: UTDateTimeRangePicker.Calendar
    property bool showSeconds: false
    property string dateDisplayFormat: "yyyy-MM-dd"
    property int  spacing: 8

    // Time pickers are visually shorter than date pickers; widen a little when
    // seconds are shown (an extra wheel column).
    property int datePickerWidth: 150
    property int timePickerWidth: showSeconds ? 130 : 96

    // === Range value (source of truth, includes time in DateTime mode) ===
    property date startDate: new Date()
    property date endDate:   new Date()

    // === Constraints ===
    property var  minDate: undefined        // Date or undefined
    property var  maxDate: undefined        // Date or undefined
    property real maximumRangeMs: 0         // 0 = unlimited

    // === Read-only output ===
    readonly property bool showTime: mode === UTDateTimeRangePicker.DateTime
    readonly property real durationMs: endDate.getTime() - startDate.getTime()
    readonly property bool valid: durationMs >= 0
    readonly property string rangeText: {
        var format = dateDisplayFormat
        if (showTime)
            format += showSeconds ? " HH:mm:ss" : " HH:mm"
        return Qt.formatDateTime(startDate, format) + "  \u2192  " + Qt.formatDateTime(endDate, format)
    }

    signal rangeChanged(var start, var end)

    implicitWidth: layout.implicitWidth
    implicitHeight: layout.implicitHeight

    // Guard so model->children sync does not re-enter as a user edit.
    property bool synchronizing: false

    // ── Date helpers ──
    function dayStart(date) {
        var normalized = new Date(date)
        normalized.setHours(0, 0, 0, 0)
        return normalized
    }
    function dayEnd(date) {
        var normalized = new Date(date)
        normalized.setHours(23, 59, 59, 999)
        return normalized
    }
    function earlierDate(first, second) {
        if (first === undefined) return second
        if (second === undefined) return first
        return first.getTime() <= second.getTime() ? first : second
    }
    function laterDate(first, second) {
        if (first === undefined) return second
        if (second === undefined) return first
        return first.getTime() >= second.getTime() ? first : second
    }

    // Combine a date (day) with a time (hours/minutes/seconds) into a full Date.
    function composeDateTime(dayPart, hours, minutes, seconds) {
        var result = new Date(dayPart)
        if (showTime)
            result.setHours(hours, minutes, seconds, 0)
        else
            result.setHours(0, 0, 0, 0)
        return result
    }

    // Push the source-of-truth values into the child pickers.
    function syncChildren() {
        synchronizing = true
        if (startDateLoader.item) startDateLoader.item.selectedDate = startDate
        if (endDateLoader.item)   endDateLoader.item.selectedDate   = endDate
        startTimePicker.hours   = startDate.getHours()
        startTimePicker.minutes = startDate.getMinutes()
        startTimePicker.seconds = startDate.getSeconds()
        endTimePicker.hours   = endDate.getHours()
        endTimePicker.minutes = endDate.getMinutes()
        endTimePicker.seconds = endDate.getSeconds()
        synchronizing = false
    }

    // Pull child-picker values back into the model, applying the auto-clamp.
    // `editedSide` is "start" or "end": the edited side is clamped to the other.
    function commitFromChildren(editedSide) {
        if (synchronizing || !startDateLoader.item || !endDateLoader.item)
            return

        var newStart = composeDateTime(startDateLoader.item.selectedDate,
                         startTimePicker.hours, startTimePicker.minutes, startTimePicker.seconds)
        var newEnd = composeDateTime(endDateLoader.item.selectedDate,
                         endTimePicker.hours, endTimePicker.minutes, endTimePicker.seconds)

        // Keep start <= end (strategy B: clamp the edited side, leave the other
        // fixed). The opposite side acts as a hard wall - this matches Ant
        // Design / MUI range pickers and the wheel "stops at the boundary" feel.
        if (newStart.getTime() > newEnd.getTime()) {
            if (editedSide === "start")
                newStart = new Date(newEnd)
            else
                newEnd = new Date(newStart)
        }

        // Enforce a maximum span, if configured.
        if (maximumRangeMs > 0 && (newEnd.getTime() - newStart.getTime()) > maximumRangeMs) {
            if (editedSide === "start")
                newEnd = new Date(newStart.getTime() + maximumRangeMs)
            else
                newStart = new Date(newEnd.getTime() - maximumRangeMs)
        }

        startDate = newStart
        endDate   = newEnd
        syncChildren()
        rangeChanged(startDate, endDate)
    }

    // External assignment of the range -> refresh the children.
    onStartDateChanged: if (!synchronizing) syncChildren()
    onEndDateChanged:   if (!synchronizing) syncChildren()
    Component.onCompleted: syncChildren()

    // Date-picker styles. Both share the BaseDatePicker API (selectedDate,
    // displayFormat, minDate, maxDate, dateChanged); `isStart` picks the
    // cross-limit side. Loaded via a Loader so the style can switch at runtime.
    Component {
        id: calendarDateComp
        UTDatePicker {
            property bool isStart: false
            anchors.fill: parent
            displayFormat: control.dateDisplayFormat
            minDate: isStart ? control.minDate
                             : control.laterDate(control.minDate, control.dayStart(control.startDate))
            maxDate: isStart ? control.earlierDate(control.maxDate, control.dayEnd(control.endDate))
                             : control.maxDate
            onDateChanged: control.commitFromChildren(isStart ? "start" : "end")
        }
    }
    Component {
        id: wheelDateComp
        UTDateWheelPicker {
            property bool isStart: false
            anchors.fill: parent
            displayFormat: control.dateDisplayFormat
            // Static overall bounds (NOT the cross-limited start/end values): the
            // wheel's year origin (minYear) tracks minDate, and cross-limiting it
            // to the live start/end would feed back through the year wheel into
            // the model (binding loop). Ordering is still enforced by
            // commitFromChildren clamping the edited side.
            minDate: control.minDate
            maxDate: control.maxDate
            onDateChanged: control.commitFromChildren(isStart ? "start" : "end")
        }
    }

    RowLayout {
        id: layout
        anchors.fill: parent
        spacing: control.spacing

        // ── Start group ──
        Loader {
            id: startDateLoader
            Layout.preferredWidth: control.datePickerWidth
            Layout.preferredHeight: item ? item.implicitHeight : 32
            sourceComponent: control.datePickerStyle === UTDateTimeRangePicker.Wheel
                             ? wheelDateComp : calendarDateComp
            onLoaded: { item.isStart = true; control.syncChildren() }
        }
        UTTimePicker {
            id: startTimePicker
            visible: control.showTime
            Layout.preferredWidth: control.timePickerWidth
            showSeconds: control.showSeconds
            onTimeChanged: control.commitFromChildren("start")
        }

        UTText {
            text: "\u2192"
            horizontalAlignment: Text.AlignHCenter
            Layout.alignment: Qt.AlignVCenter
            color: UTComponentUtil.getPlainUIColor(
                       UIColorToken.Datepicker_Text, UIColorState.Normal)
        }

        // ── End group ──
        Loader {
            id: endDateLoader
            Layout.preferredWidth: control.datePickerWidth
            Layout.preferredHeight: item ? item.implicitHeight : 32
            sourceComponent: control.datePickerStyle === UTDateTimeRangePicker.Wheel
                             ? wheelDateComp : calendarDateComp
            onLoaded: { item.isStart = false; control.syncChildren() }
        }
        UTTimePicker {
            id: endTimePicker
            visible: control.showTime
            Layout.preferredWidth: control.timePickerWidth
            showSeconds: control.showSeconds
            onTimeChanged: control.commitFromChildren("end")
        }
    }
}
