import QtQuick
import QtQuick.Controls.Basic
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTDateWheelPicker - Wheel-style date picker (year / month / day Tumblers).
 *
 * Wheel counterpart to UTDatePicker (calendar grid). Column order follows the
 * locale (year-first for zh/ja/ko). The day column tracks the selected month
 * and clamps when month/year change. Shares BaseDatePicker with UTDatePicker.
 */
BaseDatePicker {
    id: control

    // === Theme tokens ===
    property var backgroundColorEnum: UIColorToken.Datepicker_Background
    property var textColorEnum:       UIColorToken.Datepicker_Text
    property var borderColorEnum:     UIColorToken.Datepicker_Border
    property var selectedColorEnum:   UIColorToken.Datepicker_Selected_Background

    property var calendarLocale: Qt.locale(Qt.uiLanguage)

    // === Presentation (collapsed box text) ===
    property string displayFormat: "yyyy-MM-dd"
    readonly property string dateValue: Qt.formatDate(selectedDate, displayFormat)

    // === Year range (wheel bounds) ===
    // Fixed bounds so the wheel window never shifts while scrolling. Derived
    // from [minDate, maxDate] when set, otherwise a wide static window. Must NOT
    // depend on selectedDate, or picking a year would move the range (binding
    // loop) instead of the selection.
    property int minYear: minDate !== undefined ? minDate.getFullYear() : 1900
    property int maxYear: maxDate !== undefined ? maxDate.getFullYear() : 2100
    readonly property int yearCount: Math.max(1, maxYear - minYear + 1)

    // === Decomposed selected date (wheel source of truth) ===
    readonly property int curYear:  selectedDate.getFullYear()
    readonly property int curMonth: selectedDate.getMonth()   // 0-11
    readonly property int curDay:   selectedDate.getDate()
    readonly property int dayCount: daysInMonth(curYear, curMonth)

    // Column order on screen, by locale.
    readonly property var fieldOrder: {
        const lang = calendarLocale.name
        const yearFirst = lang.startsWith("zh") || lang.startsWith("ja") || lang.startsWith("ko")
        return yearFirst ? ["year", "month", "day"] : ["month", "day", "year"]
    }

    // === Field setters (clamp day to the resulting month) ===
    function setYear(y) {
        var yy = Math.max(minYear, Math.min(maxYear, y))
        var d = clampDay(yy, curMonth, curDay)
        selectDate(new Date(yy, curMonth, d))
    }
    function setMonth(m) {
        var mm = ((m % 12) + 12) % 12
        var d = clampDay(curYear, mm, curDay)
        selectDate(new Date(curYear, mm, d))
    }
    function setDay(d) {
        selectDate(new Date(curYear, curMonth, d))
    }

    // === Wheel geometry ===
    property int columnWidth: 70
    property int wheelHeight: 160
    property int visibleRows: 5
    property int wheelSpacing: 6

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

    // === Drop-down wheels ===
    popup.width: control.columnWidth * 3 + control.wheelSpacing * 2 + 16
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
            function onOpened() { if (wheelRepeater.itemAt(0)) wheelRepeater.itemAt(0).forceActiveFocus() }
            function onClosed() { control.forceActiveFocus() }
        }

        // Move keyboard focus between columns. Bubbled up from the focused
        // wheel, which leaves Left/Right unhandled on purpose.
        function moveColumn(delta) {
            var n = wheelRepeater.count
            if (n <= 0) return
            var idx = 0
            for (var i = 0; i < n; ++i)
                if (wheelRepeater.itemAt(i).activeFocus) { idx = i; break }
            wheelRepeater.itemAt(((idx + delta) % n + n) % n).forceActiveFocus()
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
            color: UTComponentUtil.getPlainUIColor(control.selectedColorEnum, UIColorState.Selected)
        }

        Row {
            anchors.centerIn: parent
            spacing: control.wheelSpacing

            Repeater {
                id: wheelRepeater
                model: control.fieldOrder

                UTWheelColumn {
                    id: col
                    required property int index
                    required property string modelData
                    readonly property string field: modelData

                    width: control.columnWidth
                    height: control.wheelHeight
                    visibleRows: control.visibleRows
                    textColorEnum: control.textColorEnum
                    borderColorEnum: control.borderColorEnum
                    borderRadius: control.borderRadius

                    // Year is bounded (no wrap); month/day wrap around.
                    wrap: field !== "year"
                    // Day uses a fixed 31-item model (never rebuilt) so the
                    // wheel doesn't scroll on month change; days the current
                    // month lacks are disabled via enabledCount.
                    valueCount: field === "year" ? control.yearCount
                              : field === "month" ? 12
                                                  : 31
                    enabledCount: field === "day" ? control.dayCount : valueCount
                    value: field === "year"  ? control.curYear - control.minYear
                         : field === "month" ? control.curMonth
                                             : control.curDay - 1

                    formatValue: field === "year"
                        ? function(i) { return "" + (control.minYear + i) }
                        : field === "month"
                            ? function(i) { return control.calendarLocale.standaloneMonthName(i, Locale.ShortFormat) }
                            : function(i) { return (i < 9 ? "0" : "") + (i + 1) }

                    onValuePicked: function(v) {
                        if (col.field === "year")
                            control.setYear(control.minYear + v)
                        else if (col.field === "month")
                            control.setMonth(v)
                        else
                            control.setDay(v + 1)
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
