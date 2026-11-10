import QtQuick
import QtQuick.Controls.Basic
import UIComponentBase 1.0

/**
 * BaseWheelColumn - Wheel behavior and rendering without resource tokens.
 *
 * Index-driven: `value` / `valuePicked` are indices into [0, valueCount).
 * Owns the delegate, pointer input and Up/Down keys. Left/Right bubble to
 * the parent to move between columns; UT supplies colors, font and focus style.
 */
Tumbler {
    id: wheel

    property int valueCount: 60
    // Selectable leading items; the rest are shown but disabled (e.g. days
    // 29-31 in a short month). Fixed valueCount means the model is never
    // rebuilt, so the wheel doesn't reset/scroll when only this shrinks.
    property int enabledCount: valueCount
    property int value: 0
    signal valuePicked(int v)

    property int visibleRows: 5
    // Maps a value index to its display string.
    property var formatValue: function(i) { return (i < 10 ? "0" : "") + i }

    property color textColor: palette.text
    property color disabledTextColor: textColor
    property int centeredPixelSize: 18
    property int offCenterPixelSize: 14

    visibleItemCount: visibleRows
    focusPolicy: Qt.StrongFocus

    // Set while we drive currentIndex ourselves, so it isn't read as a scroll.
    property bool _syncing: false

    function _apply() {
        _syncing = true
        if (model !== valueCount)
            model = valueCount
        var target = Math.max(0, Math.min(valueCount - 1, value))
        if (currentIndex !== target)
            currentIndex = target
        _syncing = false
    }
    onValueChanged: _apply()
    onValueCountChanged: _apply()
    Component.onCompleted: _apply()
    onCurrentIndexChanged: {
        if (_syncing)
            return
        // Snap a landing on a disabled tail item back to the last enabled one.
        var v = Math.max(0, Math.min(currentIndex, enabledCount - 1))
        if (v !== value)
            valuePicked(v)
        else if (currentIndex !== value)
            _apply()
    }

    Keys.onUpPressed:   function(e) { stepBy(1);  e.accepted = true }
    Keys.onDownPressed: function(e) { stepBy(-1); e.accepted = true }

    // Step one item within the enabled range, wrapping or clamping.
    function stepBy(delta) {
        var n = enabledCount
        if (n <= 0)
            return
        var base = Math.min(value, n - 1)
        if (wrap)
            valuePicked(((base + delta) % n + n) % n)
        else
            valuePicked(Math.max(0, Math.min(n - 1, base + delta)))
    }

    TapHandler {
        onTapped: wheel.forceActiveFocus()
    }

    // Handle delivered wheel events in single steps; leave press/drag to the Tumbler.
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.NoButton
        onWheel: function(event) {
            wheel.stepBy(event.angleDelta.y > 0 ? -1 : 1)
            event.accepted = true
        }
    }

    delegate: Component {
        BaseText {
            required property int index
            readonly property bool disabled: index >= wheel.enabledCount
            readonly property bool centered: Tumbler.displacement === 0
            font: {
                const style = Object.assign({}, wheel.font)
                // Set one size and one weight representation to avoid conflicting aliases.
                delete style.pointSize
                delete style.weight
                style.pixelSize = centered ? wheel.centeredPixelSize : wheel.offCenterPixelSize
                style.bold = !disabled && centered
                return Qt.font(style)
            }
            text: wheel.formatValue(index)
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: disabled ? wheel.disabledTextColor : wheel.textColor
            opacity: disabled
                     ? 0.35
                     : 1.0 - Math.min(0.6, Math.abs(Tumbler.displacement) * 0.28)
        }
    }
}
