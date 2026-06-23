import QtQuick
import QtQuick.Layouts
import UIComponentBase 1.0
import UTComponent 1.0
import UTComposite 1.0
import UIResourceLoader 1.0

/**
 * UTDateTimeRangePicker - Composite range picker built from UTDatePicker +
 * UTTimePicker pairs (a "start" group and an "end" group).
 *
 * Two modes:
 *   - Date     : pick a date interval (time pickers hidden).
 *   - DateTime : pick a date + time interval (default).
 *
 * Ordering is auto-clamped: the range is always kept valid (start <= end). When
 * one side is dragged past the other, the opposite side is pushed to follow, so
 * `startDate` is never later than `endDate`. The calendars also cross-limit each
 * other so illegal days are visually disabled.
 *
 * Usage:
 *   UTDateTimeRangePicker {
 *       mode: UTDateTimeRangePicker.DateTime
 *       startDate: new Date()
 *       endDate:   new Date()
 *       onRangeChanged: (start, end) => myModel.setRange(start, end)
 *   }
 */
Item {
    id: control

    enum Mode { Date, DateTime }

    // === Configuration ===
    property int  mode: UTDateTimeRangePicker.DateTime
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
        startDatePicker.selectedDate = startDate
        endDatePicker.selectedDate   = endDate
        startTimePicker.hours   = startDate.getHours()
        startTimePicker.minutes = startDate.getMinutes()
        startTimePicker.seconds = startDate.getSeconds()
        endTimePicker.hours   = endDate.getHours()
        endTimePicker.minutes = endDate.getMinutes()
        endTimePicker.seconds = endDate.getSeconds()
        synchronizing = false
    }

    // Pull child-picker values back into the model, applying the auto-clamp.
    // `editedSide` is "start" or "end": the edited side wins, the other is pushed.
    function commitFromChildren(editedSide) {
        if (synchronizing)
            return

        var newStart = composeDateTime(startDatePicker.selectedDate,
                         startTimePicker.hours, startTimePicker.minutes, startTimePicker.seconds)
        var newEnd = composeDateTime(endDatePicker.selectedDate,
                         endTimePicker.hours, endTimePicker.minutes, endTimePicker.seconds)

        // Keep start <= end (strategy A: push the opposite side).
        if (newStart.getTime() > newEnd.getTime()) {
            if (editedSide === "start")
                newEnd = new Date(newStart)
            else
                newStart = new Date(newEnd)
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

    RowLayout {
        id: layout
        anchors.fill: parent
        spacing: control.spacing

        // ── Start group ──
        UTDatePicker {
            id: startDatePicker
            Layout.preferredWidth: control.datePickerWidth
            displayFormat: control.dateDisplayFormat
            minDate: control.minDate
            maxDate: control.earlierDate(control.maxDate, control.dayStart(control.endDate))
            onDateChanged: control.commitFromChildren("start")
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
        UTDatePicker {
            id: endDatePicker
            Layout.preferredWidth: control.datePickerWidth
            displayFormat: control.dateDisplayFormat
            minDate: control.laterDate(control.minDate, control.dayStart(control.startDate))
            maxDate: control.maxDate
            onDateChanged: control.commitFromChildren("end")
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
