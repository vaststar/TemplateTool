import QtQuick
import QtQuick.Controls.Basic
import UIComponentBase 1.0

// Wheel-specific date behavior and rendering over the shared date-picker core.
BaseDatePicker {
    id: control

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

    property BaseWheelColumn _yearColumn: null
    property BaseWheelColumn _monthColumn: null
    property BaseWheelColumn _dayColumn: null
    // Decoration references may be null while locale-dependent columns rebuild.
    // Keep column value/model bindings owned by this component.
    readonly property BaseWheelColumn yearColumn: _yearColumn
    readonly property BaseWheelColumn monthColumn: _monthColumn
    readonly property BaseWheelColumn dayColumn: _dayColumn

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

    // === Collapsed box content: current date + indicator ===
    contentItem: Item {
        BaseText {
            font: control.font
            id: valueText
            anchors.left: parent.left
            anchors.leftMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            text: control.dateValue
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

    // === Drop-down wheels ===
    popup.width: control.columnWidth * 3 + control.wheelSpacing * 2 + 16
    popup.padding: 8
    // Let the popup hold keyboard focus so its wheel columns receive keys.
    popup.focus: true
    popup.background: Rectangle {
        radius: control.borderRadius
        color: control.backgroundColor
        border.width: 1
        border.color: control.borderColor
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
            color: control.selectedBandColor
        }

        Row {
            anchors.centerIn: parent
            spacing: control.wheelSpacing

            Repeater {
                id: wheelRepeater
                model: control.fieldOrder

                onItemAdded: function(index, item) {
                    if (item.field === "year") control._yearColumn = item
                    else if (item.field === "month") control._monthColumn = item
                    else control._dayColumn = item
                }
                onItemRemoved: function(index, item) {
                    // A removed old item must not clear a newer replacement.
                    if (control._yearColumn === item) control._yearColumn = null
                    if (control._monthColumn === item) control._monthColumn = null
                    if (control._dayColumn === item) control._dayColumn = null
                }

                BaseWheelColumn {
                    id: col
                    required property int index
                    required property string modelData
                    readonly property string field: modelData

                    width: control.columnWidth
                    height: control.wheelHeight
                    visibleRows: control.visibleRows
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
}
