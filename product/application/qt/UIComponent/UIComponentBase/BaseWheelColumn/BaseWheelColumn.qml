import QtQuick
import QtQuick.Controls.Basic

/**
 * BaseWheelColumn - Unstyled wheel column logic (Tumbler core).
 *
 * Index-driven: `value` / `valuePicked` are indices into [0, valueCount).
 * Visuals + interaction live in the UT layer (UTWheelColumn). Owns Up/Down
 * keys; Left/Right bubble to the parent to move between columns.
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
}
